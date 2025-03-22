#include "valorant_adapter.h"
#include "../utils/logger.h"

ValorantAdapter::ValorantAdapter() {
    m_gameInfo.name = "Valorant";
    m_gameInfo.modelName = "valorant_model.onnx";
    m_gameInfo.aspectRatio = 16.0f / 9.0f;
    m_gameInfo.defaultFov = 103.0f;

    m_classMapping = {
        {0, EntityType::Player},  // 敌方玩家
        {1, EntityType::Player},  // 友方玩家
        {2, EntityType::Weapon},  // 步枪
        {3, EntityType::Weapon},  // 手枪
        {4, EntityType::Weapon},  // 狙击枪
        {5, EntityType::Utility}, // 技能1
        {6, EntityType::Utility}, // 技能2
        {7, EntityType::Utility}, // 技能3
        {8, EntityType::Utility}, // 终极技能
        {9, EntityType::Item}     // 尖刺
    };

    m_targetOffsets = {
        {TargetZone::Head, {0.0f, -0.22f, 0.05f}},
        {TargetZone::Chest, {0.0f, -0.05f, 0.1f}},
        {TargetZone::Stomach, {0.0f, 0.05f, 0.1f}},
        {TargetZone::Limbs, {0.0f, 0.15f, 0.1f}}
    };

    setDistanceParams(2.0f, 10.0f, 0.85f);

    Logger::info("Valorant适配器初始化");
}

void ValorantAdapter::processGameSpecific(ProcessedGameData& gameData) {
    for (auto& player : gameData.players) {
        float estHeight = static_cast<float>(player.boundingBox.height);
        float distance = m_referenceDistance * (m_referenceHeight / estHeight);
        player.estimatedDistance = distance * m_scaleFactor;

        // Valorant具有护甲系统，根据检测结果估算血量
        if (player.hasHelmet) {
            player.estimatedHealth += 50;
        }

        // 根据武器类型调整威胁等级
        if (player.weaponType == WeaponType::Sniper) {
            player.threatLevel *= 1.7f;
        } else if (player.weaponType == WeaponType::Rifle) {
            player.threatLevel *= 1.3f;
        }

        // 根据玩家姿态调整威胁等级
        if (player.boundingBox.height < (player.boundingBox.width * 1.5f)) {
            // 可能在蹲下状态
            player.threatLevel *= 1.2f;
        }
    }

    // 识别特工类型
    identifyAgentTypes(gameData);

    // 处理尖刺状态
    processSpikeStatus(gameData, cv::Mat());

    // 更新游戏环境信息
    gameData.environment.mapName = "unknown";
    gameData.environment.roundNumber = detectRoundNumber(cv::Mat());
    gameData.environment.isMatchPoint = false;
}

bool ValorantAdapter::detectGameMode(const cv::Mat& frame) {
    if (detectCompetitiveMode(frame)) {
        Logger::info("检测到竞技模式");
        return true;
    } else if (detectUnratedMode(frame)) {
        Logger::info("检测到非竞技模式");
        return true;
    } else if (detectDeathMatchMode(frame)) {
        Logger::info("检测到死亡竞赛模式");
        return true;
    }

    return false;
}

bool ValorantAdapter::analyzeMapContext(const cv::Mat& frame) {
    std::string mapName = detectCurrentMap(frame);
    if (!mapName.empty()) {
        Logger::info("检测到地图：" + mapName);
    }

    // 检测尖刺状态
    bool spikePlanted = detectSpikePlanted(frame);
    bool isDefusing = detectDefusing(frame);

    return true;
}

bool ValorantAdapter::detectCompetitiveMode(const cv::Mat& frame) {
    // 实际项目中会实现游戏模式检测
    return false;
}

bool ValorantAdapter::detectUnratedMode(const cv::Mat& frame) {
    return false;
}

bool ValorantAdapter::detectDeathMatchMode(const cv::Mat& frame) {
    return false;
}

std::string ValorantAdapter::detectCurrentMap(const cv::Mat& frame) {
    // 简化实现，在实际项目中会进行图像识别或OCR
    return "";
}

bool ValorantAdapter::detectSpikePlanted(const cv::Mat& frame) {
    // 通过视觉识别尖刺植入状态
    return false;
}

bool ValorantAdapter::detectDefusing(const cv::Mat& frame) {
    // 识别解除尖刺状态
    return false;
}

int ValorantAdapter::detectRoundNumber(const cv::Mat& frame) {
    // 识别当前回合数
    return 0;
}

void ValorantAdapter::identifyAgentTypes(ProcessedGameData& gameData) {
    // 根据视觉特征识别不同特工
    for (auto& player : gameData.players) {
        // 实际项目中这里会根据模型检测结果区分不同特工
        // 这里简化处理
    }
}

void ValorantAdapter::processSpikeStatus(ProcessedGameData& gameData, const cv::Mat& frame) {
    // 处理尖刺状态
    gameData.gameState.isBombPlanted = detectSpikePlanted(frame);
    gameData.gameState.isDefusing = detectDefusing(frame);
}