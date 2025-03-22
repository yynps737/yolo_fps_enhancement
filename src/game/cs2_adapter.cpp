#include "cs2_adapter.h"
#include "../utils/logger.h"

CS2Adapter::CS2Adapter() {
    m_gameInfo.name = "Counter-Strike 2";
    m_gameInfo.modelName = "cs2_model.onnx";
    m_gameInfo.aspectRatio = 16.0f / 9.0f;
    m_gameInfo.defaultFov = 90.0f;

    m_classMapping = {
        {0, EntityType::Player},
        {1, EntityType::Player},
        {2, EntityType::Weapon},
        {3, EntityType::Weapon},
        {4, EntityType::Weapon},
        {5, EntityType::Utility},
        {6, EntityType::Utility},
        {7, EntityType::Utility}
    };

    m_targetOffsets = {
        {TargetZone::Head, {0.0f, -0.2f, 0.05f}},
        {TargetZone::Chest, {0.0f, -0.05f, 0.1f}},
        {TargetZone::Stomach, {0.0f, 0.05f, 0.1f}},
        {TargetZone::Limbs, {0.0f, 0.15f, 0.1f}}
    };

    setDistanceParams(2.0f, 10.0f, 0.8f);

    Logger::info("CS2适配器初始化");
}

void CS2Adapter::processGameSpecific(ProcessedGameData& gameData) {
    for (auto& player : gameData.players) {
        float estHeight = static_cast<float>(player.boundingBox.height);
        float distance = m_referenceDistance * (m_referenceHeight / estHeight);
        player.estimatedDistance = distance * m_scaleFactor;

        if (player.hasHelmet) {
            player.estimatedHealth += 20;
        }

        if (player.weaponType == WeaponType::Sniper) {
            player.threatLevel *= 1.5f;
        }
    }

    // 检测游戏状态
    gameData.gameState.isBombPlanted = false;
    gameData.gameState.isDefusing = false;

    // 更新游戏环境信息
    gameData.environment.mapName = "unknown";
    gameData.environment.roundNumber = 0;
    gameData.environment.isMatchPoint = false;
}

bool CS2Adapter::detectGameMode(const cv::Mat& frame) {
    if (detectCompetitiveMode(frame)) {
        Logger::info("检测到竞技模式");
        return true;
    } else if (detectCasualMode(frame)) {
        Logger::info("检测到休闲模式");
        return true;
    } else if (detectDeathMatchMode(frame)) {
        Logger::info("检测到死亡竞赛模式");
        return true;
    }

    return false;
}

bool CS2Adapter::analyzeMapContext(const cv::Mat& frame) {
    std::string mapName = detectCurrentMap(frame);
    if (!mapName.empty()) {
        Logger::info("检测到地图：" + mapName);
    }

    // 检测炸弹状态
    bool bombPlanted = isBombPlanted(frame);
    bool isDefusing = this->isDefusing(frame);

    return true;
}

bool CS2Adapter::detectCompetitiveMode(const cv::Mat& frame) {
    // 简化实现，实际项目中会使用模板匹配或OCR识别游戏界面元素
    return false;
}

bool CS2Adapter::detectCasualMode(const cv::Mat& frame) {
    // 简化实现
    return false;
}

bool CS2Adapter::detectDeathMatchMode(const cv::Mat& frame) {
    // 简化实现
    return false;
}

std::string CS2Adapter::detectCurrentMap(const cv::Mat& frame) {
    // 简化实现
    return "";
}

bool CS2Adapter::isBombPlanted(const cv::Mat& frame) {
    // 简化实现
    return false;
}

bool CS2Adapter::isDefusing(const cv::Mat& frame) {
    // 简化实现
    return false;
}