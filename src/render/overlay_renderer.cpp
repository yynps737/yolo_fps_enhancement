#include "overlay_renderer.h"
#include <sstream>

OverlayRenderer::OverlayRenderer()
    : m_showSettings(false),
      m_fps(0.0)
{
    m_settings.showBoundingBoxes = true;
    m_settings.showLabels = true;
    m_settings.showDistance = true;
    m_settings.showHealth = true;
    m_settings.showHeadPosition = true;
    m_settings.showLines = true;
    m_settings.showCrosshair = true;
    m_settings.showFPS = true;
    m_settings.showDebugInfo = false;

    m_settings.enemyColor = cv::Scalar(0, 0, 255);
    m_settings.allyColor = cv::Scalar(0, 255, 0);
    m_settings.itemColor = cv::Scalar(255, 255, 0);
    m_settings.targetColor = cv::Scalar(255, 0, 255);
    m_settings.crosshairColor = cv::Scalar(0, 255, 255);

    m_lastRenderTime = std::chrono::high_resolution_clock::now();
}

void OverlayRenderer::renderOverlay(cv::Mat& frame, const ProcessedGameData& gameData) {
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastRenderTime).count();
    m_lastRenderTime = currentTime;

    if (duration > 0) {
        m_fps = 1000.0 / duration;
    }

    renderEntities(frame, gameData.players);
    renderEntities(frame, gameData.items);
    renderTargets(frame, gameData.potentialTargets);

    if (m_settings.showCrosshair) {
        renderCrosshair(frame);
    }

    if (m_settings.showFPS) {
        std::stringstream ss;
        ss << "FPS: " << std::fixed << std::setprecision(1) << m_fps;
        cv::putText(frame, ss.str(), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
    }

    if (m_settings.showDebugInfo) {
        renderDebugInfo(frame, gameData);
    }

    if (m_showSettings) {
        renderSettingsWindow(frame);
    }
}

void OverlayRenderer::updateSettings(const RenderSettings& settings) {
    m_settings = settings;
}

void OverlayRenderer::showSettingsWindow(bool show) {
    m_showSettings = show;
}

void OverlayRenderer::renderEntities(cv::Mat& frame, const std::vector<Entity>& entities) {
    for (const auto& entity : entities) {
        cv::Scalar color = getEntityColor(entity);

        if (m_settings.showBoundingBoxes) {
            cv::rectangle(frame, entity.boundingBox, color, 2);
        }

        if (m_settings.showLabels) {
            std::stringstream ss;
            ss << entity.className;

            if (m_settings.showDistance && entity.estimatedDistance > 0) {
                ss << " (" << std::fixed << std::setprecision(1) << entity.estimatedDistance << "m)";
            }

            cv::putText(frame, ss.str(), cv::Point(entity.boundingBox.x, entity.boundingBox.y - 10),
                       cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 2);
        }

        if (m_settings.showHealth && entity.type == EntityType::Player) {
            int healthBarWidth = entity.boundingBox.width;
            int healthBarHeight = 5;
            cv::Rect healthBarBg(entity.boundingBox.x, entity.boundingBox.y - 20, healthBarWidth, healthBarHeight);
            cv::Rect healthBar(entity.boundingBox.x, entity.boundingBox.y - 20,
                              static_cast<int>(healthBarWidth * entity.estimatedHealth / 100.0f), healthBarHeight);

            cv::rectangle(frame, healthBarBg, cv::Scalar(80, 80, 80), -1);

            cv::Scalar healthColor;
            if (entity.estimatedHealth > 70) {
                healthColor = cv::Scalar(0, 255, 0);
            } else if (entity.estimatedHealth > 30) {
                healthColor = cv::Scalar(0, 255, 255);
            } else {
                healthColor = cv::Scalar(0, 0, 255);
            }

            cv::rectangle(frame, healthBar, healthColor, -1);
        }

        if (m_settings.showHeadPosition && entity.type == EntityType::Player) {
            int headY = entity.boundingBox.y + static_cast<int>(entity.boundingBox.height * -0.2f);
            cv::Point headPoint(entity.boundingBox.x + entity.boundingBox.width / 2, headY);
            cv::circle(frame, headPoint, 5, color, -1);
        }

        if (m_settings.showLines && entity.type == EntityType::Player) {
            cv::Point center(frame.cols / 2, frame.rows / 2);
            cv::Point entityCenter(entity.boundingBox.x + entity.boundingBox.width / 2,
                                  entity.boundingBox.y + entity.boundingBox.height / 2);

            cv::line(frame, center, entityCenter, color, 1, cv::LINE_AA);
        }
    }
}

void OverlayRenderer::renderTargets(cv::Mat& frame, const std::vector<TargetInfo>& targets) {
    if (targets.empty()) {
        return;
    }

    const auto& primaryTarget = targets[0];

    cv::circle(frame, primaryTarget.targetPoint, 10, m_settings.targetColor, 2);
    cv::line(frame, cv::Point(primaryTarget.targetPoint.x - 15, primaryTarget.targetPoint.y),
             cv::Point(primaryTarget.targetPoint.x + 15, primaryTarget.targetPoint.y),
             m_settings.targetColor, 2);
    cv::line(frame, cv::Point(primaryTarget.targetPoint.x, primaryTarget.targetPoint.y - 15),
             cv::Point(primaryTarget.targetPoint.x, primaryTarget.targetPoint.y + 15),
             m_settings.targetColor, 2);

    for (size_t i = 1; i < targets.size() && i < 5; ++i) {
        const auto& target = targets[i];
        cv::circle(frame, target.targetPoint, 5, m_settings.targetColor, 1);
    }
}

void OverlayRenderer::renderCrosshair(cv::Mat& frame) {
    int centerX = frame.cols / 2;
    int centerY = frame.rows / 2;

    cv::line(frame, cv::Point(centerX - 10, centerY), cv::Point(centerX + 10, centerY),
             m_settings.crosshairColor, 2);
    cv::line(frame, cv::Point(centerX, centerY - 10), cv::Point(centerX, centerY + 10),
             m_settings.crosshairColor, 2);

    cv::circle(frame, cv::Point(centerX, centerY), 5, m_settings.crosshairColor, 1);
}

void OverlayRenderer::renderDebugInfo(cv::Mat& frame, const ProcessedGameData& gameData) {
    std::stringstream ss;

    ss << "Players: " << gameData.players.size() << " (Allies: " << gameData.gameState.allyCount
       << ", Enemies: " << gameData.gameState.enemyCount << ")";

    cv::putText(frame, ss.str(), cv::Point(10, frame.rows - 60), cv::FONT_HERSHEY_SIMPLEX,
               0.5, cv::Scalar(255, 255, 255), 1);

    ss.str("");
    ss << "Items: " << gameData.items.size() << ", Projectiles: " << gameData.projectiles.size();

    cv::putText(frame, ss.str(), cv::Point(10, frame.rows - 40), cv::FONT_HERSHEY_SIMPLEX,
               0.5, cv::Scalar(255, 255, 255), 1);

    ss.str("");
    ss << "Targets: " << gameData.potentialTargets.size();

    cv::putText(frame, ss.str(), cv::Point(10, frame.rows - 20), cv::FONT_HERSHEY_SIMPLEX,
               0.5, cv::Scalar(255, 255, 255), 1);
}

void OverlayRenderer::renderSettingsWindow(cv::Mat& frame) {
    int windowWidth = 300;
    int windowHeight = 400;
    int windowX = frame.cols - windowWidth - 10;
    int windowY = 50;

    cv::Rect windowRect(windowX, windowY, windowWidth, windowHeight);
    cv::rectangle(frame, windowRect, cv::Scalar(50, 50, 50), -1);
    cv::rectangle(frame, windowRect, cv::Scalar(200, 200, 200), 1);

    cv::putText(frame, "Settings", cv::Point(windowX + 10, windowY + 30),
               cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);

    int lineHeight = 25;
    int startY = windowY + 60;

    std::vector<std::string> options = {
        "Bounding Boxes: " + std::string(m_settings.showBoundingBoxes ? "ON" : "OFF"),
        "Labels: " + std::string(m_settings.showLabels ? "ON" : "OFF"),
        "Distance: " + std::string(m_settings.showDistance ? "ON" : "OFF"),
        "Health: " + std::string(m_settings.showHealth ? "ON" : "OFF"),
        "Head Position: " + std::string(m_settings.showHeadPosition ? "ON" : "OFF"),
        "Lines: " + std::string(m_settings.showLines ? "ON" : "OFF"),
        "Crosshair: " + std::string(m_settings.showCrosshair ? "ON" : "OFF"),
        "FPS: " + std::string(m_settings.showFPS ? "ON" : "OFF"),
        "Debug Info: " + std::string(m_settings.showDebugInfo ? "ON" : "OFF"),
        "Aim Assist: " + std::string(m_assistSettings.aimAssistEnabled ? "ON" : "OFF"),
        "Trigger Bot: " + std::string(m_assistSettings.triggerBotEnabled ? "ON" : "OFF"),
        "Recoil Control: " + std::string(m_assistSettings.recoilControlEnabled ? "ON" : "OFF")
    };

    for (size_t i = 0; i < options.size(); ++i) {
        cv::putText(frame, options[i], cv::Point(windowX + 20, startY + lineHeight * i),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
    }

    std::stringstream ss;
    ss << "Aim Assist Strength: " << std::fixed << std::setprecision(1) << m_assistSettings.aimAssistStrength * 100.0f << "%";
    cv::putText(frame, ss.str(), cv::Point(windowX + 20, startY + lineHeight * options.size()),
               cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

    ss.str("");
    ss << "Smoothness: " << std::fixed << std::setprecision(1) << m_assistSettings.smoothness * 100.0f << "%";
    cv::putText(frame, ss.str(), cv::Point(windowX + 20, startY + lineHeight * (options.size() + 1)),
               cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
}

cv::Scalar OverlayRenderer::getEntityColor(const Entity& entity) {
    if (entity.type == EntityType::Player) {
        if (entity.team == TeamType::T || entity.team == TeamType::Enemy) {
            return m_settings.enemyColor;
        } else if (entity.team == TeamType::CT || entity.team == TeamType::Allied) {
            return m_settings.allyColor;
        }
    } else if (entity.type == EntityType::Weapon || entity.type == EntityType::Item || entity.type == EntityType::Utility) {
        return m_settings.itemColor;
    }

    return cv::Scalar(200, 200, 200);
}