#define NOMINMAX
#include "utils.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Utils {

// 字符串处理
std::string trim(const std::string& str) {
    const auto begin = str.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }

    const auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(begin, end - begin + 1);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return result;
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return result;
}

// 文件操作
bool fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    file << content;
    return file.good();
}

// 图像处理
cv::Mat resizeKeepAspectRatio(const cv::Mat& input, const cv::Size& size) {
    cv::Mat output;

    double h1 = size.width * (input.rows / (double)input.cols);
    double w1 = size.height * (input.cols / (double)input.rows);

    if (h1 <= size.height) {
        cv::resize(input, output, cv::Size(size.width, h1));
    } else {
        cv::resize(input, output, cv::Size(w1, size.height));
    }

    return output;
}

cv::Rect scaleRect(const cv::Rect& rect, float scale) {
    int newWidth = static_cast<int>(rect.width * scale);
    int newHeight = static_cast<int>(rect.height * scale);

    int offsetX = (newWidth - rect.width) / 2;
    int offsetY = (newHeight - rect.height) / 2;

    return cv::Rect(
        rect.x - offsetX,
        rect.y - offsetY,
        newWidth,
        newHeight
    );
}

// 数学工具
float calculateIOU(const cv::Rect& box1, const cv::Rect& box2) {
    int x1 = std::max(box1.x, box2.x);
    int y1 = std::max(box1.y, box2.y);
    int x2 = std::min(box1.x + box1.width, box2.x + box2.width);
    int y2 = std::min(box1.y + box1.height, box2.y + box2.height);

    if (x2 < x1 || y2 < y1) {
        return 0.0f;
    }

    float intersectionArea = (x2 - x1) * (y2 - y1);
    float box1Area = box1.width * box1.height;
    float box2Area = box2.width * box2.height;

    return intersectionArea / (box1Area + box2Area - intersectionArea);
}

float euclideanDistance(const cv::Point& p1, const cv::Point& p2) {
    return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

float normalizeValue(float value, float min, float max) {
    return (value - min) / (max - min);
}

// 系统相关
void setThreadPriority(int priority) {
#ifdef _WIN32
    SetThreadPriority(GetCurrentThread(), priority);
#endif
}

std::string getExecutablePath() {
    char buffer[MAX_PATH];

#ifdef _WIN32
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
#else
    return "./";
#endif
}

std::string getTimeStamp() {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&timeT), "%Y%m%d_%H%M%S");

    return ss.str();
}

}