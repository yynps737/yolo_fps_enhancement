#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <opencv2/opencv.hpp>

// 前向声明，避免循环包含
class Config;
class ScreenCapture;
class YoloDetector;
class DetectionProcessor;
class AssistController;
class OverlayRenderer;
class GameAdapter;
class ModelManager;
class ProcessedGameData;
struct GameContext;

#include "config.h"
#include "../capture/screen_capture.h"
#include "../detection/yolo_detector.h"
#include "../processing/detection_processor.h"
#include "../assist/assist_controller.h"
#include "../render/overlay_renderer.h"
#include "../game/game_adapter.h"
#include "../model/model_manager.h"

class App {
public:
    App();
    ~App();

    bool initialize();
    void run();
    void stop();

private:
    void processFrame(const cv::Mat& frame);
    bool switchGame(const std::string& gameName);
    bool switchModel(const std::string& modelName);

    void handleKeyboardInput();
    void updateGameContext();
    void showSettings();

    std::unique_ptr<Config> m_config;
    std::unique_ptr<ScreenCapture> m_screenCapture;
    std::unique_ptr<YoloDetector> m_detector;
    std::unique_ptr<DetectionProcessor> m_processor;
    std::unique_ptr<AssistController> m_assistController;
    std::unique_ptr<OverlayRenderer> m_renderer;
    std::unique_ptr<GameAdapter> m_gameAdapter;
    std::unique_ptr<ModelManager> m_modelManager;

    std::thread m_processingThread;
    std::atomic<bool> m_isRunning;

    GameContext m_gameContext;

    cv::Mat m_currentFrame;
    ProcessedGameData m_currentGameData;

    bool m_showSettings;
    bool m_showOverlay;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastFrameTime;
};