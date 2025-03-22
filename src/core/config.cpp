#include "config.h"
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

// 其余代码保持不变...

Config::Config(const std::string& configFilePath)
    : m_configFilePath(configFilePath)
{
    m_config.activeGame = "Counter-Strike 2";
    m_config.activeModel = "cs2_model.onnx";

    m_config.captureWidth = 1920;
    m_config.captureHeight = 1080;
    m_config.captureRate = 60;

    m_config.detectionThreshold = 0.5f;
    m_config.enableTracking = true;

    m_config.assistSettings.aimAssistEnabled = false;
    m_config.assistSettings.triggerBotEnabled = false;
    m_config.assistSettings.recoilControlEnabled = false;
    m_config.assistSettings.espEnabled = true;

    m_config.assistSettings.aimAssistStrength = 0.5f;
    m_config.assistSettings.smoothness = 0.3f;
    m_config.assistSettings.fieldOfView = 10.0f;

    m_config.renderSettings.showBoundingBoxes = true;
    m_config.renderSettings.showLabels = true;
    m_config.renderSettings.showDistance = true;
    m_config.renderSettings.showHealth = true;
    m_config.renderSettings.showHeadPosition = true;
    m_config.renderSettings.showLines = true;
    m_config.renderSettings.showCrosshair = true;
    m_config.renderSettings.showFPS = true;
    m_config.renderSettings.showDebugInfo = false;

    m_config.renderSettings.enemyColor = cv::Scalar(0, 0, 255);
    m_config.renderSettings.allyColor = cv::Scalar(0, 255, 0);
    m_config.renderSettings.itemColor = cv::Scalar(255, 255, 0);
    m_config.renderSettings.targetColor = cv::Scalar(255, 0, 255);
    m_config.renderSettings.crosshairColor = cv::Scalar(0, 255, 255);

    m_config.inputSettings.aimKey = VK_RBUTTON;
    m_config.inputSettings.triggerKey = VK_LSHIFT;

    m_config.inputSettings.hotkeyMap[HotkeyFunction::ToggleAimAssist] = VK_F1;
    m_config.inputSettings.hotkeyMap[HotkeyFunction::ToggleTriggerBot] = VK_F2;
    m_config.inputSettings.hotkeyMap[HotkeyFunction::ToggleESP] = VK_F3;
    m_config.inputSettings.hotkeyMap[HotkeyFunction::ToggleRecoilControl] = VK_F4;
    m_config.inputSettings.hotkeyMap[HotkeyFunction::IncreaseStrength] = VK_UP;
    m_config.inputSettings.hotkeyMap[HotkeyFunction::DecreaseStrength] = VK_DOWN;
}

bool Config::load() {
    try {
        std::ifstream ifs(m_configFilePath);
        if (!ifs.is_open()) {
            return false;
        }

        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document doc;
        doc.ParseStream(isw);

        if (doc.HasParseError() || !doc.IsObject()) {
            return false;
        }

        if (doc.HasMember("activeGame") && doc["activeGame"].IsString()) {
            m_config.activeGame = doc["activeGame"].GetString();
        }

        if (doc.HasMember("activeModel") && doc["activeModel"].IsString()) {
            m_config.activeModel = doc["activeModel"].GetString();
        }

        if (doc.HasMember("captureWidth") && doc["captureWidth"].IsInt()) {
            m_config.captureWidth = doc["captureWidth"].GetInt();
        }

        if (doc.HasMember("captureHeight") && doc["captureHeight"].IsInt()) {
            m_config.captureHeight = doc["captureHeight"].GetInt();
        }

        if (doc.HasMember("captureRate") && doc["captureRate"].IsInt()) {
            m_config.captureRate = doc["captureRate"].GetInt();
        }

        if (doc.HasMember("detectionThreshold") && doc["detectionThreshold"].IsFloat()) {
            m_config.detectionThreshold = doc["detectionThreshold"].GetFloat();
        }

        if (doc.HasMember("enableTracking") && doc["enableTracking"].IsBool()) {
            m_config.enableTracking = doc["enableTracking"].GetBool();
        }

        if (doc.HasMember("assistSettings") && doc["assistSettings"].IsObject()) {
            const auto& assist = doc["assistSettings"];

            if (assist.HasMember("aimAssistEnabled") && assist["aimAssistEnabled"].IsBool()) {
                m_config.assistSettings.aimAssistEnabled = assist["aimAssistEnabled"].GetBool();
            }

            if (assist.HasMember("triggerBotEnabled") && assist["triggerBotEnabled"].IsBool()) {
                m_config.assistSettings.triggerBotEnabled = assist["triggerBotEnabled"].GetBool();
            }

            if (assist.HasMember("recoilControlEnabled") && assist["recoilControlEnabled"].IsBool()) {
                m_config.assistSettings.recoilControlEnabled = assist["recoilControlEnabled"].GetBool();
            }

            if (assist.HasMember("espEnabled") && assist["espEnabled"].IsBool()) {
                m_config.assistSettings.espEnabled = assist["espEnabled"].GetBool();
            }

            if (assist.HasMember("aimAssistStrength") && assist["aimAssistStrength"].IsFloat()) {
                m_config.assistSettings.aimAssistStrength = assist["aimAssistStrength"].GetFloat();
            }

            if (assist.HasMember("smoothness") && assist["smoothness"].IsFloat()) {
                m_config.assistSettings.smoothness = assist["smoothness"].GetFloat();
            }

            if (assist.HasMember("fieldOfView") && assist["fieldOfView"].IsFloat()) {
                m_config.assistSettings.fieldOfView = assist["fieldOfView"].GetFloat();
            }
        }

        if (doc.HasMember("renderSettings") && doc["renderSettings"].IsObject()) {
            const auto& render = doc["renderSettings"];

            if (render.HasMember("showBoundingBoxes") && render["showBoundingBoxes"].IsBool()) {
                m_config.renderSettings.showBoundingBoxes = render["showBoundingBoxes"].GetBool();
            }

            if (render.HasMember("showLabels") && render["showLabels"].IsBool()) {
                m_config.renderSettings.showLabels = render["showLabels"].GetBool();
            }

            if (render.HasMember("showDistance") && render["showDistance"].IsBool()) {
                m_config.renderSettings.showDistance = render["showDistance"].GetBool();
            }

            if (render.HasMember("showHealth") && render["showHealth"].IsBool()) {
                m_config.renderSettings.showHealth = render["showHealth"].GetBool();
            }

            if (render.HasMember("showHeadPosition") && render["showHeadPosition"].IsBool()) {
                m_config.renderSettings.showHeadPosition = render["showHeadPosition"].GetBool();
            }

            if (render.HasMember("showLines") && render["showLines"].IsBool()) {
                m_config.renderSettings.showLines = render["showLines"].GetBool();
            }

            if (render.HasMember("showCrosshair") && render["showCrosshair"].IsBool()) {
                m_config.renderSettings.showCrosshair = render["showCrosshair"].GetBool();
            }

            if (render.HasMember("showFPS") && render["showFPS"].IsBool()) {
                m_config.renderSettings.showFPS = render["showFPS"].GetBool();
            }

            if (render.HasMember("showDebugInfo") && render["showDebugInfo"].IsBool()) {
                m_config.renderSettings.showDebugInfo = render["showDebugInfo"].GetBool();
            }
        }

        if (doc.HasMember("inputSettings") && doc["inputSettings"].IsObject()) {
            const auto& input = doc["inputSettings"];

            if (input.HasMember("aimKey") && input["aimKey"].IsInt()) {
                m_config.inputSettings.aimKey = input["aimKey"].GetInt();
            }

            if (input.HasMember("triggerKey") && input["triggerKey"].IsInt()) {
                m_config.inputSettings.triggerKey = input["triggerKey"].GetInt();
            }

            if (input.HasMember("hotkeyMap") && input["hotkeyMap"].IsObject()) {
                const auto& hotkeys = input["hotkeyMap"];

                for (int i = 0; i < static_cast<int>(HotkeyFunction::DecreaseStrength) + 1; ++i) {
                    std::string key = std::to_string(i);
                    if (hotkeys.HasMember(key.c_str()) && hotkeys[key.c_str()].IsInt()) {
                        m_config.inputSettings.hotkeyMap[static_cast<HotkeyFunction>(i)] = hotkeys[key.c_str()].GetInt();
                    }
                }
            }
        }

        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// 修复config.cpp文件中的save()函数
bool Config::save() {
    try {
        rapidjson::Document doc;
        doc.SetObject();
        auto& allocator = doc.GetAllocator();

        doc.AddMember("activeGame", m_config.activeGame, allocator);
        doc.AddMember("activeModel", m_config.activeModel, allocator);
        doc.AddMember("captureWidth", m_config.captureWidth, allocator);
        doc.AddMember("captureHeight", m_config.captureHeight, allocator);
        doc.AddMember("captureRate", m_config.captureRate, allocator);
        doc.AddMember("detectionThreshold", m_config.detectionThreshold, allocator);
        doc.AddMember("enableTracking", m_config.enableTracking, allocator);

        rapidjson::Value assistSettings(rapidjson::kObjectType);
        assistSettings.AddMember("aimAssistEnabled", m_config.assistSettings.aimAssistEnabled, allocator);
        assistSettings.AddMember("triggerBotEnabled", m_config.assistSettings.triggerBotEnabled, allocator);
        assistSettings.AddMember("recoilControlEnabled", m_config.assistSettings.recoilControlEnabled, allocator);
        assistSettings.AddMember("espEnabled", m_config.assistSettings.espEnabled, allocator);
        assistSettings.AddMember("aimAssistStrength", m_config.assistSettings.aimAssistStrength, allocator);
        assistSettings.AddMember("smoothness", m_config.assistSettings.smoothness, allocator);
        assistSettings.AddMember("fieldOfView", m_config.assistSettings.fieldOfView, allocator);

        doc.AddMember("assistSettings", assistSettings, allocator);

        rapidjson::Value renderSettings(rapidjson::kObjectType);
        renderSettings.AddMember("showBoundingBoxes", m_config.renderSettings.showBoundingBoxes, allocator);
        renderSettings.AddMember("showLabels", m_config.renderSettings.showLabels, allocator);
        renderSettings.AddMember("showDistance", m_config.renderSettings.showDistance, allocator);
        renderSettings.AddMember("showHealth", m_config.renderSettings.showHealth, allocator);
        renderSettings.AddMember("showHeadPosition", m_config.renderSettings.showHeadPosition, allocator);
        renderSettings.AddMember("showLines", m_config.renderSettings.showLines, allocator);
        renderSettings.AddMember("showCrosshair", m_config.renderSettings.showCrosshair, allocator);
        renderSettings.AddMember("showFPS", m_config.renderSettings.showFPS, allocator);
        renderSettings.AddMember("showDebugInfo", m_config.renderSettings.showDebugInfo, allocator);

        doc.AddMember("renderSettings", renderSettings, allocator);

        rapidjson::Value inputSettings(rapidjson::kObjectType);
        inputSettings.AddMember("aimKey", m_config.inputSettings.aimKey, allocator);
        inputSettings.AddMember("triggerKey", m_config.inputSettings.triggerKey, allocator);

        // 修复hotkeyMap处理部分
        rapidjson::Value hotkeyMap(rapidjson::kObjectType);
        for (const auto& pair : m_config.inputSettings.hotkeyMap) {
            int functionId = static_cast<int>(pair.first);
            std::string keyStr = std::to_string(functionId);

            // 创建一个临时字符串值作为键
            rapidjson::Value keyValue;
            keyValue.SetString(keyStr.c_str(), static_cast<rapidjson::SizeType>(keyStr.length()), allocator);

            // 添加到hotkeyMap
            hotkeyMap.AddMember(keyValue, pair.second, allocator);
        }

        inputSettings.AddMember("hotkeyMap", hotkeyMap, allocator);
        doc.AddMember("inputSettings", inputSettings, allocator);

        std::ofstream ofs(m_configFilePath);
        if (!ofs.is_open()) {
            return false;
        }

        rapidjson::OStreamWrapper osw(ofs);
        rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
        doc.Accept(writer);

        return true;
    } catch (const std::exception&) {
        return false;
    }
}

AppConfig& Config::getConfig() {
    return m_config;
}