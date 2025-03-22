#include "app.h"
#include "../utils/logger.h"
#include "../game/cs2_adapter.h"
#include "../game/valorant_adapter.h"
#include "../game/battlefield_adapter.h"
#include "../game/l4d2_adapter.h"
#include "../game/cod_adapter.h"
#include <filesystem>
#include <chrono>

App::App()
    : m_isRunning(false),
      m_showSettings(false),
      m_showOverlay(true)
{
    m_config = std::make_unique<Config>();
    m_screenCapture = std::make_unique<ScreenCapture>();
    m_detector = std::make_unique<YoloDetector>("", false);
    m_processor = std::make_unique<DetectionProcessor>();
    m_assistController = std::make_unique<AssistController>();
    m_renderer = std::make_unique<OverlayRenderer>();
    m_modelManager = std::make_unique<ModelManager>("./models");

    m_gameContext.fieldOfView = 90.0f;
    m_gameContext.screenSize = cv::Size(1920, 1080);
    m_gameContext.aspectRatio = 16.0f / 9.0f;
    m_gameContext.isAiming = false;
}

App::~App() {
    stop();
}

bool App::initialize() {
    if (!std::filesystem::exists("./models")) {
        std::filesystem::create_directory("./models");
    }

    if (!std::filesystem::exists("./resources")) {
        std::filesystem::create_directory("./resources");
    }

    Logger::info("初始化应用程序");

    if (!m_config->load()) {
        Logger::warning("无法加载配置文件，使用默认设置");
        if (!m_config->save()) {
            Logger::error("无法保存配置文件");
        }
    }

    auto& appConfig = m_config->getConfig();

    m_screenCapture->setCaptureBounds(0, 0, appConfig.captureWidth, appConfig.captureHeight);
    m_screenCapture->setFullscreenCapture(true);

    if (!switchGame(appConfig.activeGame)) {
        Logger::warning("无法加载游戏适配器：" + appConfig.activeGame + "，使用通用适配器");
    }

    if (!switchModel(appConfig.activeModel)) {
        Logger::error("无法加载模型：" + appConfig.activeModel);
        return false;
    }

    m_processor->enableTracking(appConfig.enableTracking);

    m_assistController->initialize(appConfig.inputSettings);
    m_assistController->updateSettings(appConfig.assistSettings);

    m_renderer->updateSettings(appConfig.renderSettings);

    m_gameContext.screenSize = cv::Size(appConfig.captureWidth, appConfig.captureHeight);
    m_gameContext.aspectRatio = static_cast<float>(appConfig.captureWidth) / appConfig.captureHeight;

    Logger::info("初始化完成");

    return true;
}

void App::run() {
    if (m_isRunning.load()) {
        return;
    }

    Logger::info("启动应用程序");

    m_isRunning.store(true);
    m_screenCapture->startCapture();

    m_lastFrameTime = std::chrono::high_resolution_clock::now();

    m_processingThread = std::thread([this]() {
        try {
            while (m_isRunning.load()) {
                auto startTime = std::chrono::high_resolution_clock::now();

                m_currentFrame = m_screenCapture->getLatestFrame();
                if (!m_currentFrame.empty()) {
                    processFrame(m_currentFrame);
                }

                handleKeyboardInput();

                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

                int sleepTime = std::max(1, 1000 / 60 - static_cast<int>(duration));
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            }
        } catch (const std::exception& e) {
            Logger::error("处理线程异常：" + std::string(e.what()));
        }
    });
}

void App::stop() {
    if (!m_isRunning.load()) {
        return;
    }

    Logger::info("停止应用程序");

    m_isRunning.store(false);
    m_screenCapture->stopCapture();

    if (m_processingThread.joinable()) {
        m_processingThread.join();
    }

    m_config->save();
}

void App::processFrame(const cv::Mat& frame) {
    if (frame.empty()) {
        return;
    }

    auto& appConfig = m_config->getConfig();
    updateGameContext();

    if (m_gameAdapter) {
        m_gameAdapter->analyzeMapContext(frame);
    }

    std::vector<Detection> detections = m_detector->detect(frame, appConfig.detectionThreshold);

    m_currentGameData = m_processor->processDetections(detections, m_gameContext);

    if (m_gameAdapter) {
        m_gameAdapter->processGameSpecific(m_currentGameData);
    }

    m_assistController->processGameData(m_currentGameData);

    if (m_showOverlay) {
        cv::Mat overlay = frame.clone();
        m_renderer->renderOverlay(overlay, m_currentGameData);

        try {
            cv::imshow("YOLO FPS辅助", overlay);
            cv::waitKey(1);
        } catch (const cv::Exception& e) {
            Logger::error("渲染异常：" + std::string(e.what()));
        }
    }
}

bool App::switchGame(const std::string& gameName) {
    if (gameName == "Counter-Strike 2") {
        m_gameAdapter = std::make_unique<CS2Adapter>();
        Logger::info("加载CS2游戏适配器");
        return true;
    } else if (gameName == "Valorant") {
        m_gameAdapter = std::make_unique<ValorantAdapter>();
        Logger::info("加载Valorant游戏适配器");
        return true;
    } else if (gameName == "Battlefield") {
        m_gameAdapter = std::make_unique<BattlefieldAdapter>();
        Logger::info("加载Battlefield游戏适配器");
        return true;
    } else if (gameName == "Left 4 Dead 2") {
        m_gameAdapter = std::make_unique<L4D2Adapter>();
        Logger::info("加载L4D2游戏适配器");
        return true;
    } else if (gameName == "Call of Duty") {
        m_gameAdapter = std::make_unique<CODAdapter>();
        Logger::info("加载COD游戏适配器");
        return true;
    }

    return false;
}

bool App::switchModel(const std::string& modelName) {
    std::string modelPath = "./models/" + modelName;
    if (!std::filesystem::exists(modelPath)) {
        Logger::error("模型文件不存在：" + modelPath);

        // 尝试查找任何可用的模型
        auto availableModels = m_modelManager->getAvailableModels();
        if (!availableModels.empty()) {
            std::string fallbackModel = "./models/" + availableModels[0].name;
            Logger::warning("尝试加载备用模型：" + fallbackModel);
            return m_detector->loadModel(fallbackModel);
        }

        return false;
    }

    if (m_detector->loadModel(modelPath)) {
        Logger::info("加载模型：" + modelName);

        // 如果有游戏适配器，更新其类别映射
        if (m_gameAdapter) {
            m_processor->setClassMapping(m_gameAdapter->getClassMapping());
        }

        return true;
    }

    return false;
}

void App::handleKeyboardInput() {
    bool settingsKeyPressed = m_assistController->isKeyPressed(VK_F10);
    static bool settingsKeyWasPressed = false;

    if (settingsKeyPressed && !settingsKeyWasPressed) {
        m_showSettings = !m_showSettings;
        m_renderer->showSettingsWindow(m_showSettings);
    }

    settingsKeyWasPressed = settingsKeyPressed;

    bool overlayKeyPressed = m_assistController->isKeyPressed(VK_F11);
    static bool overlayKeyWasPressed = false;

    if (overlayKeyPressed && !overlayKeyWasPressed) {
        m_showOverlay = !m_showOverlay;

        if (!m_showOverlay) {
            cv::destroyAllWindows();
        }
    }

    overlayKeyWasPressed = overlayKeyPressed;

    bool exitKeyPressed = m_assistController->isKeyPressed(VK_F12);
    static bool exitKeyWasPressed = false;

    if (exitKeyPressed && !exitKeyWasPressed) {
        stop();
    }

    exitKeyWasPressed = exitKeyPressed;
}

void App::updateGameContext() {
    m_gameContext.isAiming = m_assistController->isKeyPressed(m_config->getConfig().inputSettings.aimKey);
}