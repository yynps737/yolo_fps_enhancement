#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <future>
#include <opencv2/opencv.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

class ScreenCapture {
public:
    ScreenCapture(int captureRate = 60);
    ~ScreenCapture();
    
    void setCaptureBounds(int x, int y, int width, int height);
    void setFullscreenCapture(bool fullscreen);
    
    cv::Mat getLatestFrame();
    std::future<cv::Mat> captureFrameAsync();
    
    void startCapture();
    void stopCapture();
    
    double getAverageFPS() const;
    
private:
    void captureThreadFunction();
    
#ifdef _WIN32
    void captureWithDirectX();
    void captureWithGDI();
#endif
    
    void captureWithOpenGL();
    
    std::atomic<bool> m_isRunning;
    std::mutex m_frameMutex;
    cv::Mat m_latestFrame;
    std::thread m_captureThread;
    
    int m_captureRate;
    bool m_fullscreen;
    
#ifdef _WIN32
    RECT m_captureBounds;
#else
    cv::Rect m_captureBounds;
#endif
    
    std::vector<double> m_frameTimes;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastCaptureTime;
};