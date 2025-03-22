#include "yolo_detector.h"
#include <fstream>
#include <filesystem>

YoloDetector::YoloDetector(const std::string& modelPath, bool useGPU)
    : m_inputSize(640, 640),
      m_deviceType(useGPU ? DeviceType::CUDA : DeviceType::CPU),
      m_nmsEnabled(true),
      m_iouThreshold(0.45),
      m_confThreshold(0.5),
      m_batchEnabled(false)
#ifdef WITH_ONNX
      , m_env(ORT_LOGGING_LEVEL_WARNING, "YoloDetector"),
      m_memoryInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault))
#endif
{
    loadModel(modelPath);
}

YoloDetector::~YoloDetector() {
#ifdef WITH_ONNX
    for (auto& name : m_inputNames) {
        free(const_cast<char*>(name));
    }
    for (auto& name : m_outputNames) {
        free(const_cast<char*>(name));
    }
#endif
}

bool YoloDetector::loadModel(const std::string& modelPath) {
#ifdef WITH_ONNX
    try {
        Ort::SessionOptions sessionOptions;

        switch (m_deviceType) {
            case DeviceType::CPU:
                break;
            case DeviceType::CUDA:
                sessionOptions.AppendExecutionProvider_CUDA(0);
                break;
            case DeviceType::DirectML:
                #ifdef WITH_DIRECTML
                sessionOptions.AppendExecutionProvider_DML(0);
                #endif
                break;
            case DeviceType::TensorRT:
                #ifdef WITH_TENSORRT
                sessionOptions.AppendExecutionProvider_Tensorrt(0);
                #endif
                break;
        }

        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        m_session = std::make_unique<Ort::Session>(m_env, modelPath.c_str(), sessionOptions);

        Ort::AllocatorWithDefaultOptions allocator;

        size_t numInputNodes = m_session->GetInputCount();
        m_inputNames.resize(numInputNodes);

        for (size_t i = 0; i < numInputNodes; i++) {
            char* inputName = m_session->GetInputName(i, allocator);
            m_inputNames[i] = inputName;
        }

        size_t numOutputNodes = m_session->GetOutputCount();
        m_outputNames.resize(numOutputNodes);

        for (size_t i = 0; i < numOutputNodes; i++) {
            char* outputName = m_session->GetOutputName(i, allocator);
            m_outputNames[i] = outputName;
        }

        std::string classNamesPath = modelPath.substr(0, modelPath.find_last_of('.')) + ".names";
        if (std::filesystem::exists(classNamesPath)) {
            m_classNames = loadClassNames(classNamesPath);
        } else {
            std::vector<std::string> defaultClasses = {
                "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat"
            };
            m_classNames = defaultClasses;
        }

        return true;
    } catch (const Ort::Exception& e) {
        return false;
    }
#else
    return false;
#endif
}

std::vector<std::string> YoloDetector::getAvailableModels() const {
    std::vector<std::string> models;

    std::string modelsDir = "./models";
    for (const auto& entry : std::filesystem::directory_iterator(modelsDir)) {
        if (entry.path().extension() == ".onnx") {
            models.push_back(entry.path().filename().string());
        }
    }

    return models;
}

std::vector<Detection> YoloDetector::detect(const cv::Mat& frame, float confThreshold) {
    m_confThreshold = confThreshold;

#ifdef WITH_ONNX
    try {
        cv::Mat blob;
        cv::resize(frame, blob, m_inputSize);
        cv::cvtColor(blob, blob, cv::COLOR_BGR2RGB);

        blob.convertTo(blob, CV_32F, 1.0/255.0);

        std::vector<float> inputTensorValues(blob.ptr<float>(0),
                                           blob.ptr<float>(0) + blob.rows * blob.cols * blob.channels());

        std::vector<int64_t> inputShape = {1, 3, static_cast<int64_t>(m_inputSize.height), static_cast<int64_t>(m_inputSize.width)};

        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
            m_memoryInfo, inputTensorValues.data(), inputTensorValues.size(),
            inputShape.data(), inputShape.size()
        );

        std::vector<Ort::Value> outputTensors = m_session->Run(
            Ort::RunOptions{nullptr},
            m_inputNames.data(),
            &inputTensor,
            1,
            m_outputNames.data(),
            m_outputNames.size()
        );

        std::vector<cv::Mat> outputs;

        for (auto& tensor : outputTensors) {
            auto typeInfo = tensor.GetTypeInfo();
            auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
            auto shape = tensorInfo.GetShape();

            size_t outputCount = 1;
            for (size_t i = 0; i < shape.size(); i++) {
                outputCount *= shape[i];
            }

            float* floatData = tensor.GetTensorMutableData<float>();
            cv::Mat output(shape[1], shape[2], CV_32F, floatData);
            outputs.push_back(output);
        }

        return postprocess(frame, outputs);
    } catch (const Ort::Exception& e) {
        return {};
    }
#else
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
    std::vector<std::string> classNames;
    std::ifstream file(filename);
    if (!file.is_open()) {
        return classNames;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            classNames.push_back(line);
        }
    }

    return classNames;
}

std::vector<Detection> YoloDetector::postprocess(const cv::Mat& frame, const std::vector<cv::Mat>& outputs) {
    std::vector<Detection> detections;
    std::vector<int> classIds;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;

    for (const auto& output : outputs) {
        for (int i = 0; i < output.rows; ++i) {
            float confidence = output.at<float>(i, 4);

            if (confidence > m_confThreshold) {
                cv::Mat scores = output.row(i).colRange(5, output.cols);
                cv::Point classIdPoint;
                double maxScore;

                cv::minMaxLoc(scores, nullptr, &maxScore, nullptr, &classIdPoint);

                if (maxScore > m_confThreshold) {
                    int centerX = static_cast<int>(output.at<float>(i, 0));
                    int centerY = static_cast<int>(output.at<float>(i, 1));
                    int width = static_cast<int>(output.at<float>(i, 2));
                    int height = static_cast<int>(output.at<float>(i, 3));

                    int left = centerX - width / 2;
                    int top = centerY - height / 2;

                    classIds.push_back(classIdPoint.x);
                    confidences.push_back(static_cast<float>(maxScore * confidence));
                    boxes.push_back(cv::Rect(left, top, width, height));
                }
            }
        }
    }

    if (m_nmsEnabled) {
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, m_confThreshold, m_iouThreshold, indices);

        for (size_t i = 0; i < indices.size(); ++i) {
            int idx = indices[i];
            Detection detection;

            float scaleX = static_cast<float>(frame.cols) / m_inputSize.width;
            float scaleY = static_cast<float>(frame.rows) / m_inputSize.height;

            detection.boundingBox = cv::Rect(
                static_cast<int>(boxes[idx].x * scaleX),
                static_cast<int>(boxes[idx].y * scaleY),
                static_cast<int>(boxes[idx].width * scaleX),
                static_cast<int>(boxes[idx].height * scaleY)
            );

            detection.classId = classIds[idx];
            if (detection.classId < m_classNames.size()) {
                detection.className = m_classNames[detection.classId];
            } else {
                detection.className = "unknown";
            }

            detection.confidence = confidences[idx];

            detections.push_back(detection);
        }
    }

    return detections;
}