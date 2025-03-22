#pragma once

#include <map>
#include <functional>
#include "../processing/entity.h"

enum class HotkeyFunction {
    ToggleAimAssist,
    ToggleTriggerBot,
    ToggleESP,
    ToggleRecoilControl,
    CycleTargetMode,
    IncreaseStrength,
    DecreaseStrength
};

struct InputSettings {
    int aimKey;
    int triggerKey;
    std::map<HotkeyFunction, int> hotkeyMap;
};

struct AssistSettings {
    bool aimAssistEnabled;
    bool triggerBotEnabled;
    bool recoilControlEnabled;
    bool espEnabled;

    float aimAssistStrength;
    float smoothness;
    float fieldOfView;

    float recoilControlX;
    float recoilControlY;
};

class AssistController {
public:
    AssistController();

    void initialize(const InputSettings& settings);
    void updateSettings(const AssistSettings& settings);

    void processGameData(const ProcessedGameData& gameData);
    void applyAimAssist(const TargetInfo& target);

    void enableAimAssist(bool enable);
    void enableTriggerBot(bool enable);
    void enableRecoilControl(bool enable);

    void registerHotkey(HotkeyFunction function, int keyCode);
    void processInput();

    bool isKeyPressed(int keyCode) const;

private:
    void moveMouse(int deltaX, int deltaY, float smoothing);
    void simulateMouseClick(bool isDown);

    TargetInfo selectBestTarget(const std::vector<TargetInfo>& targets);
    float calculateAssistStrength(const TargetInfo& target);
    Vec2 calculateAimVector(const TargetInfo& target);

    AssistSettings m_settings;
    InputSettings m_inputSettings;

    TargetInfo m_currentTarget;
    bool m_aimAssistActive;
    bool m_triggerActive;

    std::map<int, bool> m_keyStates;
    ProcessedGameData m_lastGameData;

    int m_screenCenterX;
    int m_screenCenterY;
};