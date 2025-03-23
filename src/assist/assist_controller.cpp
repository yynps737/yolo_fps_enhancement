#define NOMINMAX

#include "assist_controller.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif

AssistController::AssistController()
    : m_aimAssistActive(false),
      m_triggerActive(false),
      m_screenCenterX(1920 / 2),
      m_screenCenterY(1080 / 2)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    m_settings.aimAssistEnabled = false;
    m_settings.triggerBotEnabled = false;
    m_settings.recoilControlEnabled = false;
    m_settings.espEnabled = true;

    m_settings.aimAssistStrength = 0.5f;
    m_settings.smoothness = 0.3f;
    m_settings.fieldOfView = 10.0f;

    m_settings.recoilControlX = 0.7f;
    m_settings.recoilControlY = 0.7f;

    m_inputSettings.aimKey = VK_RBUTTON;
    m_inputSettings.triggerKey = VK_LSHIFT;
}

void AssistController::initialize(const InputSettings& settings) {
    m_inputSettings = settings;
}

void AssistController::updateSettings(const AssistSettings& settings) {
    m_settings = settings;
}

void AssistController::processGameData(const ProcessedGameData& gameData) {
    m_lastGameData = gameData;

    processInput();

    if (m_settings.aimAssistEnabled && isKeyPressed(m_inputSettings.aimKey)) {
        if (!gameData.potentialTargets.empty()) {
            auto target = selectBestTarget(gameData.potentialTargets);
            applyAimAssist(target);
        }
    } else {
        m_aimAssistActive = false;
    }

    if (m_settings.triggerBotEnabled && isKeyPressed(m_inputSettings.triggerKey)) {
        bool shouldTrigger = false;

        for (const auto& target : gameData.potentialTargets) {
            if (std::abs(target.aimVector.x) < 5 && std::abs(target.aimVector.y) < 5) {
                shouldTrigger = true;
                break;
            }
        }

        if (shouldTrigger && !m_triggerActive) {
            simulateMouseClick(true);
            m_triggerActive = true;
        } else if (!shouldTrigger && m_triggerActive) {
            simulateMouseClick(false);
            m_triggerActive = false;
        }
    } else if (m_triggerActive) {
        simulateMouseClick(false);
        m_triggerActive = false;
    }
}

void AssistController::applyAimAssist(const TargetInfo& target) {
    if (target.entityId < 0) {
        return;
    }

    m_currentTarget = target;
    m_aimAssistActive = true;

    float strength = calculateAssistStrength(target);

    int deltaX = static_cast<int>(target.aimVector.x * strength);
    int deltaY = static_cast<int>(target.aimVector.y * strength);

    if (std::abs(deltaX) < 1 && std::abs(deltaY) < 1) {
        return;
    }

    if (m_settings.recoilControlEnabled) {
        deltaY = static_cast<int>(deltaY * m_settings.recoilControlY);
        deltaX = static_cast<int>(deltaX * m_settings.recoilControlX);
    }

    moveMouse(deltaX, deltaY, m_settings.smoothness);
}

void AssistController::enableAimAssist(bool enable) {
    m_settings.aimAssistEnabled = enable;
}

void AssistController::enableTriggerBot(bool enable) {
    m_settings.triggerBotEnabled = enable;
    if (!enable && m_triggerActive) {
        simulateMouseClick(false);
        m_triggerActive = false;
    }
}

void AssistController::enableRecoilControl(bool enable) {
    m_settings.recoilControlEnabled = enable;
}

void AssistController::registerHotkey(HotkeyFunction function, int keyCode) {
    m_inputSettings.hotkeyMap[function] = keyCode;
}

void AssistController::processInput() {
    for (const auto& [function, keyCode] : m_inputSettings.hotkeyMap) {
        bool wasPressed = m_keyStates[keyCode];
        bool isPressed = isKeyPressed(keyCode);

        if (isPressed && !wasPressed) {
            switch (function) {
                case HotkeyFunction::ToggleAimAssist:
                    m_settings.aimAssistEnabled = !m_settings.aimAssistEnabled;
                    break;

                case HotkeyFunction::ToggleTriggerBot:
                    enableTriggerBot(!m_settings.triggerBotEnabled);
                    break;

                case HotkeyFunction::ToggleESP:
                    m_settings.espEnabled = !m_settings.espEnabled;
                    break;

                case HotkeyFunction::ToggleRecoilControl:
                    m_settings.recoilControlEnabled = !m_settings.recoilControlEnabled;
                    break;

                case HotkeyFunction::IncreaseStrength:
                    m_settings.aimAssistStrength = std::min(1.0f, m_settings.aimAssistStrength + 0.1f);
                    break;

                case HotkeyFunction::DecreaseStrength:
                    m_settings.aimAssistStrength = std::max(0.0f, m_settings.aimAssistStrength - 0.1f);
                    break;
            }
        }

        m_keyStates[keyCode] = isPressed;
    }
}

bool AssistController::isKeyPressed(int keyCode) const {
#ifdef _WIN32
    return (GetAsyncKeyState(keyCode) & 0x8000) != 0;
#else
    return false;
#endif
}

void AssistController::moveMouse(int deltaX, int deltaY, float smoothing) {
#ifdef _WIN32
    if (std::abs(deltaX) < 1 && std::abs(deltaY) < 1) {
        return;
    }

    float smoothFactor = std::min(0.9f, std::max(0.1f, smoothing));

    float strength = 1.0f - smoothFactor;
    int finalDeltaX = static_cast<int>(deltaX * strength * (0.8f + 0.2f * (std::abs(deltaX) / 100.0f)));
    int finalDeltaY = static_cast<int>(deltaY * strength * (0.8f + 0.2f * (std::abs(deltaY) / 100.0f)));

    if (std::abs(finalDeltaX) > 3) {
        finalDeltaX += (std::rand() % 3) - 1;
    }
    if (std::abs(finalDeltaY) > 3) {
        finalDeltaY += (std::rand() % 3) - 1;
    }

    INPUT input;
    ZeroMemory(&input, sizeof(INPUT));

    input.type = INPUT_MOUSE;
    input.mi.dx = finalDeltaX;
    input.mi.dy = finalDeltaY;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;

    SendInput(1, &input, sizeof(INPUT));
#endif
}

void AssistController::simulateMouseClick(bool isDown) {
#ifdef _WIN32
    INPUT input;
    ZeroMemory(&input, sizeof(INPUT));

    input.type = INPUT_MOUSE;
    input.mi.dwFlags = isDown ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;

    SendInput(1, &input, sizeof(INPUT));
#endif
}

TargetInfo AssistController::selectBestTarget(const std::vector<TargetInfo>& targets) {
    if (targets.empty()) {
        TargetInfo emptyTarget;
        emptyTarget.entityId = -1;
        emptyTarget.priority = 0;
        return emptyTarget;
    }

    std::vector<TargetInfo> filteredTargets;

    for (const auto& target : targets) {
        if (std::abs(target.aimVector.x) < m_settings.fieldOfView * 10 &&
            std::abs(target.aimVector.y) < m_settings.fieldOfView * 10) {
            filteredTargets.push_back(target);
        }
    }

    if (filteredTargets.empty()) {
        return targets[0];
    }

    std::sort(filteredTargets.begin(), filteredTargets.end(), [](const TargetInfo& a, const TargetInfo& b) {
        return a.priority > b.priority;
    });

    return filteredTargets[0];
}

float AssistController::calculateAssistStrength(const TargetInfo& target) {
    float baseStrength = m_settings.aimAssistStrength;

    float distanceFactor = 1.0f - std::min(1.0f, target.distance / 100.0f) * 0.3f;

    float aimVectorLength = std::sqrt(target.aimVector.x * target.aimVector.x + target.aimVector.y * target.aimVector.y);
    float aimFactor = 1.0f - std::min(1.0f, aimVectorLength / 400.0f) * 0.7f;

    float finalStrength = baseStrength * distanceFactor * aimFactor;

    return std::min(1.0f, finalStrength);
}

Vec2 AssistController::calculateAimVector(const TargetInfo& target) {
    return target.aimVector;
}