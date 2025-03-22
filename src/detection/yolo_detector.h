#pragma once

#include <string>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>

#ifdef WITH_ONNX
#include <onnxruntime/core/session/onnxruntime_cxx_api.h>
#endif

enum class DeviceType {
    CPU,
    CUDA,
    DirectML,
    TensorRT
};

struct Detection {
    cv::Rect boundingBox;
    int classId;
    std::string className;
    float confidence;
    std::vector<cv::Point> keypoints;
};

class YoloDetector {
public:
    YoloDetector(const std::string& modelPath, bool useGPU = true);
    ~YoloDetector();
    
    bool loadModel(const std::string& modelPath);
    std::vector<std::string> getAvailableModels() const;
    std::vector<Detection> detect(const cv::Mat& frame, float confThreshold = 0.5);
    
    std::vector<std::string> getClassNames() const;
    void setInputSize(int width, int height);
    void enableNMS(bool enable, float iouThreshold = 0.45);
    
    void setDevicePreference(DeviceType device);
    void enableBatchProcessing(bool enable);
    
private:
    std::vector<std::string> loadClassNames(const std::string& filename);
    std::vector<Detection> postprocess(const cv::Mat& frame, const std::vector<cv::Mat>& outputs);
    
#ifdef WITH_ONNX
    std::unique_ptr<Ort::Session> m_session;
    Ort::Env m_env;
    Ort::MemoryInfo m_memoryInfo;
    std::vector<const char*> m_inputNames;
    std::vector<const char*> m_outputNames;
#endif
    
    cv::Size m_inputSize;
    std::vector<std::string> m_classNames;
    DeviceType m_deviceType;
    
    bool m_nmsEnabled;
    float m_iouThreshold;
    float m_confThreshold;
    
    bool m_batchEnabled;
    std::vector<cv::Mat> m_batchFrames;
};