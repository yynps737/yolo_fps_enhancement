#define NOMINMAX

#include "screen_capture.h"
#include <numeric>

ScreenCapture::ScreenCapture(int captureRate)
    : m_isRunning(false),
      m_captureRate(captureRate),
      m_fullscreen(true)
{
#ifdef _WIN32
    m_captureBounds = {0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
#else
    m_captureBounds = cv::Rect(0, 0, 1920, 1080);
#endif
    m_frameTimes.reserve(100);
}

ScreenCapture::~ScreenCapture() {
    stopCapture();
}

void ScreenCapture::setCaptureBounds(int x, int y, int width, int height) {
#ifdef _WIN32
    m_captureBounds = {x, y, x + width, y + height};
#else
    m_captureBounds = cv::Rect(x, y, width, height);
#endif
}

void ScreenCapture::setFullscreenCapture(bool fullscreen) {
    m_fullscreen = fullscreen;
    if (fullscreen) {
#ifdef _WIN32
        m_captureBounds = {0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)};
#else
        m_captureBounds = cv::Rect(0, 0, 1920, 1080);
#endif
    }
}

void ScreenCapture::startCapture() {
    if (!m_isRunning.load()) {
        m_isRunning.store(true);
        m_captureThread = std::thread(&ScreenCapture::captureThreadFunction, this);
    }
}

void ScreenCapture::stopCapture() {
    if (m_isRunning.load()) {
        m_isRunning.store(false);
        if (m_captureThread.joinable()) {
            m_captureThread.join();
        }
    }
}

cv::Mat ScreenCapture::getLatestFrame() {
    std::lock_guard<std::mutex> lock(m_frameMutex);
    return m_latestFrame.clone();
}

std::future<cv::Mat> ScreenCapture::captureFrameAsync() {
    return std::async(std::launch::async, [this]() {
        return getLatestFrame();
    });
}

double ScreenCapture::getAverageFPS() const {
    if (m_frameTimes.empty()) {
        return 0.0;
    }

    double avgTime = std::accumulate(m_frameTimes.begin(), m_frameTimes.end(), 0.0) / m_frameTimes.size();
    return avgTime > 0 ? 1000.0 / avgTime : 0.0;
}

void ScreenCapture::captureThreadFunction() {
    while (m_isRunning.load()) {
        auto startTime = std::chrono::high_resolution_clock::now();

#ifdef _WIN32
        captureWithDirectX();
#else
        captureWithOpenGL();
#endif

        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        m_frameTimes.push_back(duration);
        if (m_frameTimes.size() > 100) {
            m_frameTimes.erase(m_frameTimes.begin());
        }

        int sleepTime = std::max(0, 1000 / m_captureRate - static_cast<int>(duration));
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    }
}

#ifdef _WIN32
void ScreenCapture::captureWithDirectX() {
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen);

    int width = m_captureBounds.right - m_captureBounds.left;
    int height = m_captureBounds.bottom - m_captureBounds.top;

    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);
    HGDIOBJ oldObject = SelectObject(hdcMemDC, hbmScreen);

    BitBlt(hdcMemDC, 0, 0, width, height, hdcScreen, m_captureBounds.left, m_captureBounds.top, SRCCOPY);

    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    cv::Mat frame(height, width, CV_8UC3);
    GetDIBits(hdcMemDC, hbmScreen, 0, height, frame.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        m_latestFrame = frame;
    }

    SelectObject(hdcMemDC, oldObject);
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
}

void ScreenCapture::captureWithGDI() {
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen);

    int width = m_captureBounds.right - m_captureBounds.left;
    int height = m_captureBounds.bottom - m_captureBounds.top;

    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, width, height);
    HGDIOBJ oldObject = SelectObject(hdcMemDC, hbmScreen);

    BitBlt(hdcMemDC, 0, 0, width, height, hdcScreen, m_captureBounds.left, m_captureBounds.top, SRCCOPY);

    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    cv::Mat frame(height, width, CV_8UC3);
    GetDIBits(hdcMemDC, hbmScreen, 0, height, frame.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        m_latestFrame = frame;
    }

    SelectObject(hdcMemDC, oldObject);
    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
}
#endif

void ScreenCapture::captureWithOpenGL() {
#ifdef __linux__
    // Linux下的屏幕捕获实现
    try {
        // 这里使用X11或XCB实现屏幕捕获
        Display* display = XOpenDisplay(nullptr);
        if (!display) {
            // 如果无法打开显示，创建一个空帧
            cv::Mat frame(m_captureBounds.height, m_captureBounds.width, CV_8UC3, cv::Scalar(0, 0, 0));

            std::lock_guard<std::mutex> lock(m_frameMutex);
            m_latestFrame = frame;
            return;
        }

        Window root = DefaultRootWindow(display);
        XWindowAttributes attributes;
        XGetWindowAttributes(display, root, &attributes);

        int x = m_captureBounds.x;
        int y = m_captureBounds.y;
        int width = m_captureBounds.width;
        int height = m_captureBounds.height;

        // 确保区域在屏幕范围内
        if (width <= 0) width = attributes.width;
        if (height <= 0) height = attributes.height;

        // 捕获屏幕
        XImage* img = XGetImage(display, root, x, y, width, height, AllPlanes, ZPixmap);
        if (img) {
            // 转换为OpenCV格式
            cv::Mat frame(height, width, CV_8UC4);

            // 复制像素数据
            for (int j = 0; j < height; j++) {
                for (int i = 0; i < width; i++) {
                    unsigned long pixel = XGetPixel(img, i, j);
                    frame.at<cv::Vec4b>(j, i)[0] = (pixel & img->blue_mask) >> 0;
                    frame.at<cv::Vec4b>(j, i)[1] = (pixel & img->green_mask) >> 8;
                    frame.at<cv::Vec4b>(j, i)[2] = (pixel & img->red_mask) >> 16;
                    frame.at<cv::Vec4b>(j, i)[3] = 255;
                }
            }

            // 转换颜色空间
            cv::cvtColor(frame, frame, cv::COLOR_RGBA2RGB);

            // 更新最新帧
            {
                std::lock_guard<std::mutex> lock(m_frameMutex);
                m_latestFrame = frame;
            }

            // 释放XImage
            XDestroyImage(img);
        } else {
            // 如果捕获失败，创建一个空帧
            cv::Mat frame(height, width, CV_8UC3, cv::Scalar(0, 0, 0));

            std::lock_guard<std::mutex> lock(m_frameMutex);
            m_latestFrame = frame;
        }

        // 关闭显示连接
        XCloseDisplay(display);
    } catch (const std::exception& e) {
        // 异常处理：创建一个空帧
        cv::Mat frame(m_captureBounds.height, m_captureBounds.width, CV_8UC3, cv::Scalar(0, 0, 0));

        std::lock_guard<std::mutex> lock(m_frameMutex);
        m_latestFrame = frame;
    }
#elif defined(__APPLE__)
    // macOS下的屏幕捕获实现
    // 注意：这里仅是占位符，实际实现需要使用macOS特定的API
    cv::Mat frame(m_captureBounds.height, m_captureBounds.width, CV_8UC3, cv::Scalar(0, 0, 0));

    std::lock_guard<std::mutex> lock(m_frameMutex);
    m_latestFrame = frame;
#else
    // 其他平台创建一个空帧
    cv::Mat frame(m_captureBounds.bottom, m_captureBounds.bottom, CV_8UC3, cv::Scalar(0, 0, 0));
    
    std::lock_guard<std::mutex> lock(m_frameMutex);
    m_latestFrame = frame;
#endif
}