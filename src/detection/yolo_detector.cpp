#include "yolo_detector.h"
#include "../utils/logger.h"
#include <fstream>
#include <algorithm>

YoloDetector::YoloDetector(const std::string& modelPath, bool useGPU)
    : m_inputSize(640, 640),
      m_deviceType(useGPU ? DeviceType::CUDA : DeviceType::CPU),
      m_nmsEnabled(true),
      m_iouThreshold(0.45f),
      m_confThreshold(0.5f),
      m_batchEnabled(false)
{
#ifdef WITH_ONNX
    // 初始化ONNX Runtime环境
    m_env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "YoloDetector");

    // 设置内存信息
    m_memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
#endif

    // 如果提供了模型路径，加载模型
    if (!modelPath.empty()) {
        loadModel(modelPath);
    }

    Logger::info("YoloDetector初始化完成");
}

YoloDetector::~YoloDetector() {
    Logger::debug("YoloDetector释放资源");
}

bool YoloDetector::loadModel(const std::string& modelPath) {
    try {
        Logger::info("加载YOLO模型: " + modelPath);

#ifdef WITH_ONNX
        // 配置ONNX运行时选项
        Ort::SessionOptions sessionOptions;

        // 根据设备类型设置执行提供者
        if (m_deviceType == DeviceType::CUDA) {
            #ifdef WITH_CUDA
            // 启用CUDA加速
            OrtCUDAProviderOptions cudaOptions;
            cudaOptions.device_id = 0;
            sessionOptions.AppendExecutionProvider_CUDA(cudaOptions);
            Logger::info("启用CUDA加速");
            #else
            Logger::warning("CUDA支持未编译，使用CPU执行");
            #endif
        } else if (m_deviceType == DeviceType::DirectML) {
            #ifdef WITH_DIRECTML
            // 启用DirectML加速
            sessionOptions.AppendExecutionProvider_DML(0);
            Logger::info("启用DirectML加速");
            #else
            Logger::warning("DirectML支持未编译，使用CPU执行");
            #endif
        } else if (m_deviceType == DeviceType::TensorRT) {
            #ifdef WITH_TENSORRT
            // 启用TensorRT加速
            OrtTensorRTProviderOptions trtOptions;
            sessionOptions.AppendExecutionProvider_TensorRT(trtOptions);
            Logger::info("启用TensorRT加速");
            #else
            Logger::warning("TensorRT支持未编译，使用CPU执行");
            #endif
        }

        // 启用内存优化
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        // 创建会话
        m_session = std::make_unique<Ort::Session>(m_env, modelPath.c_str(), sessionOptions);

        // 获取输入和输出信息
        Ort::AllocatorWithDefaultOptions allocator;

        // 提取输入名称和形状
        size_t numInputNodes = m_session->GetInputCount();
        m_inputNames.resize(numInputNodes);

        for (size_t i = 0; i < numInputNodes; i++) {
            auto inputName = m_session->GetInputNameAllocated(i, allocator);
            m_inputNames[i] = inputName.get();

            auto typeInfo = m_session->GetInputTypeInfo(i);
            auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
            m_inputShape = tensorInfo.GetShape();

            // 调整输入尺寸
            if (m_inputShape.size() == 4) {
                m_inputSize.width = m_inputShape[2];
                m_inputSize.height = m_inputShape[3];
            }
        }

        // 提取输出名称
        size_t numOutputNodes = m_session->GetOutputCount();
        m_outputNames.resize(numOutputNodes);

        for (size_t i = 0; i < numOutputNodes; i++) {
            auto outputName = m_session->GetOutputNameAllocated(i, allocator);
            m_outputNames[i] = outputName.get();
        }

        // 加载类别名称
        std::string classFile = modelPath.substr(0, modelPath.find_last_of('.')) + ".names";
        m_classNames = loadClassNames(classFile);

        Logger::info("模型加载成功: 输入尺寸=" + std::to_string(m_inputSize.width) + "x" +
                    std::to_string(m_inputSize.height) + ", 类别数=" + std::to_string(m_classNames.size()));

        return true;
#else
        Logger::error("ONNX Runtime支持未编译，无法加载模型");
        return false;
#endif
    } catch (const std::exception& e) {
        Logger::error("加载模型失败: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::string> YoloDetector::getAvailableModels() const {
    std::vector<std::string> modelFiles;

    // 在实际项目中，这里应该扫描models目录
    // 简化实现，返回空列表
    return modelFiles;
}

std::vector<Detection> YoloDetector::detect(const cv::Mat& frame, float confThreshold) {
    if (frame.empty()) {
        Logger::warning("检测帧为空");
        return {};
    }

    m_confThreshold = confThreshold;

#ifdef WITH_ONNX
    if (!m_session) {
        Logger::error("模型未加载");
        return {};
    }

    try {
        // 调整图像大小并进行预处理
        cv::Mat blob;
        cv::dnn::blobFromImage(frame, blob, 1.0 / 255.0, m_inputSize, cv::Scalar(), true, false, CV_32F);

        // 创建输入tensor
        auto memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        std::vector<float> inputTensorValues(blob.ptr<float>(), blob.ptr<float>() + blob.total());

        // 设置输入维度
        std::vector<int64_t> inputDims = {1, 3, m_inputSize.height, m_inputSize.width};

        // 创建输入tensor
        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            memoryInfo, inputTensorValues.data(), inputTensorValues.size(),
            inputDims.data(), inputDims.size()
        );

        // 运行推理
        auto outputTensors = m_session->Run(
            Ort::RunOptions{nullptr},
            m_inputNames.data(),
            &inputTensor,
            1,
            m_outputNames.data(),
            m_outputNames.size()
        );

        // 处理输出tensor
        std::vector<cv::Mat> outputs;

        for (auto& tensor : outputTensors) {
            auto typeInfo = tensor.GetTypeInfo();
            auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
            auto dims = tensorInfo.GetShape();

            // 获取tensor数据
            float* data = tensor.GetTensorMutableData<float>();

            // 创建OpenCV Mat并复制数据
            cv::Mat outputMat;
            if (dims.size() == 3) {
                // YOLOv8输出格式 [1, 84, num_boxes]
                outputMat = cv::Mat(dims[1], dims[2], CV_32F, data);
            } else if (dims.size() == 2) {
                // 其他可能的输出格式
                outputMat = cv::Mat(dims[0], dims[1], CV_32F, data);
            }

            outputs.push_back(outputMat);
        }

        // 后处理检测结果
        return postprocess(frame, outputs);
    } catch (const Ort::Exception& e) {
        Logger::error("ONNX运行时异常: " + std::string(e.what()));
        return {};
    } catch (const std::exception& e) {
        Logger::error("检测异常: " + std::string(e.what()));
        return {};
    }
#else
    Logger::error("ONNX Runtime支持未编译，无法执行检测");
    return {};
#endif
}

std::vector<std::string> YoloDetector::getClassNames() const {
    return m_classNames;
}

void YoloDetector::setInputSize(int width, int height) {
    m_inputSize = cv::Size(width, height);
}

void YoloDetector::enableNMS(bool enable, float iouThreshold) {
    m_nmsEnabled = enable;
    m_iouThreshold = iouThreshold;
}

void YoloDetector::setDevicePreference(DeviceType device) {
    m_deviceType = device;
}

void YoloDetector::enableBatchProcessing(bool enable) {
    m_batchEnabled = enable;
}

std::vector<std::string> YoloDetector::loadClassNames(const std::string& filename) {
    std::vector<std::string> classes;

    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                classes.push_back(line);
            }
        }
        file.close();
    } else {
        Logger::warning("无法打开类别文件: " + filename + "，使用默认类别");
        // 提供一些默认类别
        classes = {"player", "enemy", "weapon", "item", "utility"};
    }

    return classes;
}

std::vector<Detection> YoloDetector::postprocess(const cv::Mat& frame, const std::vector<cv::Mat>& outputs) {
    std::vector<Detection> detections;

    // 判断YOLOv8输出格式
    if (outputs.empty()) {
        return detections;
    }

    const cv::Mat& output = outputs[0];

    // YOLOv8输出格式为 [num_classes+4, num_boxes]
    int num_classes = output.rows - 4;
    int num_boxes = output.cols;

    if (num_classes <= 0 || num_boxes <= 0) {
        return detections;
    }

    // 遍历所有检测框
    for (int i = 0; i < num_boxes; i++) {
        // 框坐标 (x, y, w, h)
        float x = output.at<float>(0, i);
        float y = output.at<float>(1, i);
        float w = output.at<float>(2, i);
        float h = output.at<float>(3, i);

        // 查找最高置信度的类别
        int class_id = 0;
        float max_conf = -1;

        for (int j = 0; j < num_classes; j++) {
            float conf = output.at<float>(4 + j, i);
            if (conf > max_conf) {
                max_conf = conf;
                class_id = j;
            }
        }

        // 过滤低置信度检测
        if (max_conf >= m_confThreshold) {
            // 将normalized box转换为像素坐标
            int img_width = frame.cols;
            int img_height = frame.rows;

            int left = static_cast<int>((x - w / 2) * img_width);
            int top = static_cast<int>((y - h / 2) * img_height);
            int width = static_cast<int>(w * img_width);
            int height = static_cast<int>(h * img_height);

            // 确保坐标在图像范围内
            left = std::max(0, std::min(left, img_width - 1));
            top = std::max(0, std::min(top, img_height - 1));
            width = std::max(1, std::min(width, img_width - left));
            height = std::max(1, std::min(height, img_height - top));

            // 创建检测对象
            Detection det;
            det.boundingBox = cv::Rect(left, top, width, height);
            det.classId = class_id;
            det.className = (class_id < m_classNames.size()) ? m_classNames[class_id] : "unknown";
            det.confidence = max_conf;

            detections.push_back(det);
        }
    }

    // 应用非极大值抑制
    if (m_nmsEnabled && !detections.empty()) {
        std::vector<int> indices;
        std::vector<cv::Rect> boxes;
        std::vector<float> scores;

        for (const auto& det : detections) {
            boxes.push_back(det.boundingBox);
            scores.push_back(det.confidence);
        }

        cv::dnn::NMSBoxes(boxes, scores, m_confThreshold, m_iouThreshold, indices);

        std::vector<Detection> nms_detections;
        for (int idx : indices) {
            nms_detections.push_back(detections[idx]);
        }

        return nms_detections;
    }

    return detections;
}