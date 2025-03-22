#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

namespace Utils {
    // 字符串处理
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string toLower(const std::string& str);
    std::string toUpper(const std::string& str);

    // 文件操作
    bool fileExists(const std::string& path);
    std::string readFile(const std::string& path);
    bool writeFile(const std::string& path, const std::string& content);

    // 图像处理
    cv::Mat resizeKeepAspectRatio(const cv::Mat& input, const cv::Size& size);
    cv::Rect scaleRect(const cv::Rect& rect, float scale);

    // 数学工具
    float calculateIOU(const cv::Rect& box1, const cv::Rect& box2);
    float euclideanDistance(const cv::Point& p1, const cv::Point& p2);
    float normalizeValue(float value, float min, float max);

    // 系统相关
    void setThreadPriority(int priority);
    std::string getExecutablePath();
    std::string getTimeStamp();
}