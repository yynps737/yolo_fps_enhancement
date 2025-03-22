#pragma once

#include <string>
#include <map>
#include "../assist/assist_controller.h"
#include "../render/overlay_renderer.h"

struct AppConfig {
    std::string activeGame;
    std::string activeModel;

    int captureWidth;
    int captureHeight;
    int captureRate;

    float detectionThreshold;
    bool enableTracking;

    AssistSettings assistSettings;
    RenderSettings renderSettings;
    InputSettings inputSettings;
};

class Config {
public:
    Config(const std::string& configFilePath = "resources/config.json");

    bool load();
    bool save();

    AppConfig& getConfig();

private:
    std::string m_configFilePath;
    AppConfig m_config;
};