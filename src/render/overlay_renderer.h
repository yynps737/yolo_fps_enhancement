#pragma once

#include <opencv2/opencv.hpp>
#include "../processing/entity.h"
#include "../assist/assist_controller.h"

struct RenderSettings {
    bool showBoundingBoxes;
    bool showLabels;
    bool showDistance;
    bool showHealth;
    bool showHeadPosition;
    bool showLines;
    bool showCrosshair;
    bool showFPS;
    bool showDebugInfo;

    cv::Scalar enemyColor;
    cv::Scalar allyColor;
    cv::Scalar itemColor;
    cv::Scalar targetColor;
    cv::Scalar crosshairColor;
};

class OverlayRenderer {
public:
    OverlayRenderer();

    void renderOverlay(cv::Mat& frame, const ProcessedGameData& gameData);
    void updateSettings(const RenderSettings& settings);
    void showSettingsWindow(bool show);

private:
    void renderEntities(cv::Mat& frame, const std::vector<Entity>& entities);
    void renderTargets(cv::Mat& frame, const std::vector<TargetInfo>& targets);
    void renderCrosshair(cv::Mat& frame);
    void renderDebugInfo(cv::Mat& frame, const ProcessedGameData& gameData);
    void renderSettingsWindow(cv::Mat& frame);

    cv::Scalar getEntityColor(const Entity& entity);

    RenderSettings m_settings;
    bool m_showSettings;
    AssistSettings m_assistSettings;

    double m_fps;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastRenderTime;
};