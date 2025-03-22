#include "l4d2_adapter.h"
#include "../utils/logger.h"

L4D2Adapter::L4D2Adapter() {
    m_gameInfo.name = "Left 4 Dead 2";
    m_gameInfo.modelName = "l4d2_model.onnx";
    m_gameInfo.aspectRatio = 16.0f / 9.0f;
    m_gameInfo.defaultFov = 90.0f;
    
    m_classMapping = {
        {0, EntityType::Player},  // 幸存者
        {1, EntityType::Player},  // 普通感染者
        {2, EntityType::Player},  // Boomer
        {3, EntityType::Player},  // Hunter
        {4, EntityType::Player},  // Smoker
        {5, EntityType::Player},  // Tank
        {6, EntityType::Player},  // Witch
        {7, EntityType::Player},  // Spitter
        {8, EntityType::Player},  // Jockey
        {9, EntityType::Player},  // Charger
        {10, EntityType::Weapon}, // 主武器
        {11, EntityType::Weapon}, // 副武器
        {12, EntityType::Item},   // 医疗包
        {13, EntityType::Item},   // 药丸
        {14, EntityType::Item},   // 肾上腺素
        {15, EntityType::Item},   // 弹药堆
        {16, EntityType::Item},   // 投掷武器
        {17, EntityType::Item}    // 特殊物品
    };
    
    m_targetOffsets = {
        {TargetZone::Head, {0.0f, -0.25f, 0.05f}},
        {TargetZone::Chest, {0.0f, -0.05f, 0.1f}},
        {TargetZone::Stomach, {0.0f, 0.05f, 0.1f}},
        {TargetZone::Limbs, {0.0f, 0.15f, 0.1f}}
    };
    
    setDistanceParams(2.2f, 8.0f, 0.75f);
    
    Logger::info("L4D2适配器初始化");
}

void L4D2Adapter::processGameSpecific(ProcessedGameData& gameData) {
    // 处理幸存者和特殊感染者信息
    identifyZombieTypes(gameData);
    
    // 处理幸存者状态
    processSurvivorStatus(gameData);
    
    // 处理尸潮事件
    updateHordeInfo(gameData, cv::Mat());
    
    for (auto& player : gameData.players) {
        // 计算距离
        float estHeight = static_cast<float>(player.boundingBox.height);
        float distance = m_referenceDistance * (m_referenceHeight / estHeight);
        player.estimatedDistance = distance * m_scaleFactor;
        
        // 识别特殊感染者并调整威胁等级
        ZombieType zombieType = classifyZombieType(player);
        if (zombieType != ZombieType::Unknown) {
            player.threatLevel = calculateZombieThreat(player, zombieType);
        }
    }
    
    // 更新游戏环境信息
    gameData.environment.mapName = detectCurrentMap(cv::Mat());
    gameData.environment.roundNumber = 0;
    gameData.environment.isMatchPoint = false;
    
    // 游戏特有状态
    gameData.gameState.isBombPlanted = detectHordeEvent(cv::Mat());
    gameData.gameState.isDefusing = detectTankPresent(cv::Mat());
}

bool L4D2Adapter::detectGameMode(const cv::Mat& frame) {
    if (detectCampaignMode(frame)) {
        Logger::info("检测到战役模式");
        return true;
    } else if (detectVersusMode(frame)) {
        Logger::info("检测到对抗模式");
        return true;
    } else if (detectSurvivalMode(frame)) {
        Logger::info("检测到生存模式");
        return true;
    } else if (detectScavengeMode(frame)) {
        Logger::info("检测到清道夫模式");
        return true;
    }
    
    return false;
}

bool L4D2Adapter::analyzeMapContext(const cv::Mat& frame) {
    std::string mapName = detectCurrentMap(frame);
    if (!mapName.empty()) {
        Logger::info("检测到地图：" + mapName);
    }
    
    bool hordeEvent = detectHordeEvent(frame);
    if (hordeEvent) {
        Logger::info("检测到尸潮事件");
    }
    
    bool tankPresent = detectTankPresent(frame);
    if (tankPresent) {
        Logger::info("检测到Tank出现");
    }
    
    return true;
}

bool L4D2Adapter::detectCampaignMode(const cv::Mat& frame) {
    // 实际项目中会实现更复杂的检测逻辑
    return false;
}

bool L4D2Adapter::detectVersusMode(const cv::Mat& frame) {
    return false;
}

bool L4D2Adapter::detectSurvivalMode(const cv::Mat& frame) {
    return false;
}

bool L4D2Adapter::detectScavengeMode(const cv::Mat& frame) {
    return false;
}

std::string L4D2Adapter::detectCurrentMap(const cv::Mat& frame) {
    // 实际项目中会实现地图识别功能
    return "";
}

bool L4D2Adapter::detectHordeEvent(const cv::Mat& frame) {
    // 检测尸潮事件（通过音乐或视觉提示）
    return false;
}

bool L4D2Adapter::detectTankPresent(const cv::Mat& frame) {
    // 检测Tank是否存在
    return false;
}

void L4D2Adapter::identifyZombieTypes(ProcessedGameData& gameData) {
    for (auto& player : gameData.players) {
        ZombieType type = classifyZombieType(player);
        
        // 实际项目中会设置更多特殊属性
        switch (type) {
            case ZombieType::Tank:
                player.estimatedHealth = 4000;
                break;
            case ZombieType::Witch:
                player.estimatedHealth = 1000;
                break;
            case ZombieType::Boomer:
            case ZombieType::Spitter:
            case ZombieType::Smoker:
                player.estimatedHealth = 250;
                break;
            case ZombieType::Hunter:
            case ZombieType::Jockey:
            case ZombieType::Charger:
                player.estimatedHealth = 325;
                break;
            case ZombieType::Common:
                player.estimatedHealth = 50;
                break;
            default:
                break;
        }
    }
}

void L4D2Adapter::processSurvivorStatus(ProcessedGameData& gameData) {
    // 处理幸存者状态（生命值、状态效果等）
}

void L4D2Adapter::updateHordeInfo(ProcessedGameData& gameData, const cv::Mat& frame) {
    // 更新尸潮信息
}

ZombieType L4D2Adapter::classifyZombieType(const Entity& entity) {
    // 根据类名和视觉特征识别僵尸类型
    if (entity.className.find("tank") != std::string::npos) {
        return ZombieType::Tank;
    } else if (entity.className.find("witch") != std::string::npos) {
        return ZombieType::Witch;
    } else if (entity.className.find("boomer") != std::string::npos) {
        return ZombieType::Boomer;
    } else if (entity.className.find("hunter") != std::string::npos) {
        return ZombieType::Hunter;
    } else if (entity.className.find("smoker") != std::string::npos) {
        return ZombieType::Smoker;
    } else if (entity.className.find("spitter") != std::string::npos) {
        return ZombieType::Spitter;
    } else if (entity.className.find("jockey") != std::string::npos) {
        return ZombieType::Jockey;
    } else if (entity.className.find("charger") != std::string::npos) {
        return ZombieType::Charger;
    } else if (entity.className.find("common") != std::string::npos || 
               entity.className.find("infected") != std::string::npos) {
        return ZombieType::Common;
    }
    
    return ZombieType::Unknown;
}

float L4D2Adapter::calculateZombieThreat(const Entity& entity, ZombieType type) {
    float baseThreat = 10.0f / (entity.estimatedDistance + 1.0f);
    
    switch (type) {
        case ZombieType::Tank:
            return baseThreat * 5.0f;
        case ZombieType::Witch:
            return baseThreat * 4.0f;
        case ZombieType::Hunter:
        case ZombieType::Charger:
            return baseThreat * 2.5f;
        case ZombieType::Boomer:
        case ZombieType::Smoker:
        case ZombieType::Spitter:
        case ZombieType::Jockey:
            return baseThreat * 2.0f;
        case ZombieType::Common:
            return baseThreat * 0.5f;
        default:
            return baseThreat;
    }
}