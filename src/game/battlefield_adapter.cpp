#include "battlefield_adapter.h"
#include "../utils/logger.h"

BattlefieldAdapter::BattlefieldAdapter() {
    m_gameInfo.name = "Battlefield";
    m_gameInfo.modelName = "battlefield_model.onnx";
    m_gameInfo.aspectRatio = 16.0f / 9.0f;
    m_gameInfo.defaultFov = 74.0f;
    
    m_classMapping = {
        {0, EntityType::Player},  // 敌方士兵
        {1, EntityType::Player},  // 友方士兵
        {2, EntityType::Player},  // 敌方突击兵
        {3, EntityType::Player},  // 敌方工程兵
        {4, EntityType::Player},  // 敌方支援兵
        {5, EntityType::Player},  // 敌方侦察兵
        {6, EntityType::Player},  // 敌方载具驾驶员
        {7, EntityType::Weapon},  // 主武器
        {8, EntityType::Weapon},  // 副武器
        {9, EntityType::Item},    // 医疗包
        {10, EntityType::Item},   // 弹药包
        {11, EntityType::Item},   // 修理工具
        {12, EntityType::Item},   // 炸药
        {13, EntityType::Player}, // 坦克
        {14, EntityType::Player}, // 吉普车
        {15, EntityType::Player}, // 装甲车
        {16, EntityType::Player}, // 直升机
        {17, EntityType::Player}, // 战机
        {18, EntityType::Player}  // 船艇
    };
    
    m_targetOffsets = {
        {TargetZone::Head, {0.0f, -0.2f, 0.05f}},
        {TargetZone::Chest, {0.0f, -0.05f, 0.1f}},
        {TargetZone::Stomach, {0.0f, 0.05f, 0.1f}},
        {TargetZone::Limbs, {0.0f, 0.15f, 0.1f}}
    };
    
    setDistanceParams(2.0f, 15.0f, 0.9f);
    
    Logger::info("战地适配器初始化");
}

void BattlefieldAdapter::processGameSpecific(ProcessedGameData& gameData) {
    // 识别士兵兵种
    identifySoldierClasses(gameData);
    
    // 处理载具
    processVehicles(gameData);
    
    // 计算目标优先级
    calculateObjectivePriorities(gameData);
    
    // 处理玩家特定信息
    for (auto& player : gameData.players) {
        // 根据兵种和视觉特征调整距离计算
        float distanceScale = calculateDistanceScale(cv::Mat());
        
        float estHeight = static_cast<float>(player.boundingBox.height);
        float distance = m_referenceDistance * (m_referenceHeight / estHeight);
        player.estimatedDistance = distance * m_scaleFactor * distanceScale;
        
        // 识别兵种并调整威胁等级
        SoldierClass soldierClass = classifySoldierType(player);
        if (soldierClass != SoldierClass::Unknown) {
            player.threatLevel = calculateSoldierThreat(player, soldierClass);
        }
        
        // 识别载具并调整威胁等级
        VehicleType vehicleType = classifyVehicleType(player);
        if (vehicleType != VehicleType::Unknown) {
            player.threatLevel = calculateVehicleThreat(player, vehicleType);
            
            // 载具通常有更多生命值
            switch (vehicleType) {
                case VehicleType::Tank:
                    player.estimatedHealth = 1000;
                    break;
                case VehicleType::APC:
                    player.estimatedHealth = 750;
                    break;
                case VehicleType::Helicopter:
                case VehicleType::Jet:
                    player.estimatedHealth = 500;
                    break;
                case VehicleType::Jeep:
                case VehicleType::Boat:
                    player.estimatedHealth = 300;
                    break;
                default:
                    break;
            }
        }
    }
    
    // 更新游戏环境信息
    gameData.environment.mapName = detectCurrentMap(cv::Mat());
    gameData.environment.roundNumber = 0;
    gameData.environment.isMatchPoint = false;
}

bool BattlefieldAdapter::detectGameMode(const cv::Mat& frame) {
    if (detectConquestMode(frame)) {
        Logger::info("检测到征服模式");
        return true;
    } else if (detectRushMode(frame)) {
        Logger::info("检测到突袭模式");
        return true;
    } else if (detectTDMMode(frame)) {
        Logger::info("检测到团队死斗模式");
        return true;
    } else if (detectDominationMode(frame)) {
        Logger::info("检测到支配模式");
        return true;
    }
    
    return false;
}

bool BattlefieldAdapter::analyzeMapContext(const cv::Mat& frame) {
    std::string mapName = detectCurrentMap(frame);
    if (!mapName.empty()) {
        Logger::info("检测到地图：" + mapName);
    }
    
    int ticketCount = detectTicketCount(frame);
    if (ticketCount > 0) {
        Logger::info("剩余票数：" + std::to_string(ticketCount));
    }
    
    int flagStatus = detectFlagStatus(frame);
    if (flagStatus > 0) {
        Logger::info("控制点状态：" + std::to_string(flagStatus));
    }
    
    return true;
}

bool BattlefieldAdapter::detectConquestMode(const cv::Mat& frame) {
    // 实际项目中会实现复杂检测
    return false;
}

bool BattlefieldAdapter::detectRushMode(const cv::Mat& frame) {
    return false;
}

bool BattlefieldAdapter::detectTDMMode(const cv::Mat& frame) {
    return false;
}

bool BattlefieldAdapter::detectDominationMode(const cv::Mat& frame) {
    return false;
}

std::string BattlefieldAdapter::detectCurrentMap(const cv::Mat& frame) {
    // 实际项目中会实现地图识别
    return "";
}

int BattlefieldAdapter::detectTicketCount(const cv::Mat& frame) {
    // 实际项目中会使用OCR识别票数
    return 0;
}

int BattlefieldAdapter::detectFlagStatus(const cv::Mat& frame) {
    // 检测旗帜状态
    return 0;
}

bool BattlefieldAdapter::detectObjectiveStatus(const cv::Mat& frame) {
    // 检测目标状态
    return false;
}

void BattlefieldAdapter::identifySoldierClasses(ProcessedGameData& gameData) {
    for (auto& player : gameData.players) {
        SoldierClass soldierClass = classifySoldierType(player);
        
        // 根据兵种设置不同属性
        switch (soldierClass) {
            case SoldierClass::Assault:
                // 突击兵通常携带医疗包
                break;
            case SoldierClass::Engineer:
                // 工程兵可以修理载具和放置爆炸物
                break;
            case SoldierClass::Support:
                // 支援兵可以补给弹药
                break;
            case SoldierClass::Recon:
                // 侦察兵通常使用狙击枪
                player.weaponType = WeaponType::Sniper;
                break;
            default:
                break;
        }
    }
}

void BattlefieldAdapter::processVehicles(ProcessedGameData& gameData) {
    // 处理载具信息
    for (auto& player : gameData.players) {
        VehicleType vehicleType = classifyVehicleType(player);
        if (vehicleType != VehicleType::Unknown) {
            // 设置载具特定属性
        }
    }
}

void BattlefieldAdapter::calculateObjectivePriorities(ProcessedGameData& gameData) {
    // 计算目标优先级（旗帜、MCOM等）
}

SoldierClass BattlefieldAdapter::classifySoldierType(const Entity& entity) {
    // 根据类名和视觉特征识别士兵兵种
    if (entity.className.find("assault") != std::string::npos) {
        return SoldierClass::Assault;
    } else if (entity.className.find("engineer") != std::string::npos) {
        return SoldierClass::Engineer;
    } else if (entity.className.find("support") != std::string::npos) {
        return SoldierClass::Support;
    } else if (entity.className.find("recon") != std::string::npos || 
               entity.className.find("sniper") != std::string::npos) {
        return SoldierClass::Recon;
    }
    
    return SoldierClass::Unknown;
}

VehicleType BattlefieldAdapter::classifyVehicleType(const Entity& entity) {
    // 根据类名和视觉特征识别载具类型
    if (entity.className.find("tank") != std::string::npos) {
        return VehicleType::Tank;
    } else if (entity.className.find("jeep") != std::string::npos) {
        return VehicleType::Jeep;
    } else if (entity.className.find("apc") != std::string::npos) {
        return VehicleType::APC;
    } else if (entity.className.find("helicopter") != std::string::npos ||
               entity.className.find("heli") != std::string::npos) {
        return VehicleType::Helicopter;
    } else if (entity.className.find("jet") != std::string::npos) {
        return VehicleType::Jet;
    } else if (entity.className.find("boat") != std::string::npos) {
        return VehicleType::Boat;
    }
    
    return VehicleType::Unknown;
}

float BattlefieldAdapter::calculateDistanceScale(const cv::Mat& frame) {
    // 因战地地图大小差异很大，这里添加一个距离缩放因子
    return 1.0f;
}

float BattlefieldAdapter::calculateSoldierThreat(const Entity& entity, SoldierClass type) {
    float baseThreat = 10.0f / (entity.estimatedDistance + 1.0f);
    
    switch (type) {
        case SoldierClass::Recon:
            return baseThreat * (entity.estimatedDistance > 50.0f ? 2.0f : 1.2f);
        case SoldierClass::Assault:
            return baseThreat * 1.5f;
        case SoldierClass::Engineer:
            return baseThreat * 1.3f;
        case SoldierClass::Support:
            return baseThreat * 1.4f;
        default:
            return baseThreat;
    }
}

float BattlefieldAdapter::calculateVehicleThreat(const Entity& entity, VehicleType type) {
    float baseThreat = 10.0f / (entity.estimatedDistance + 1.0f);
    
    switch (type) {
        case VehicleType::Tank:
            return baseThreat * 4.0f;
        case VehicleType::APC:
            return baseThreat * 3.0f;
        case VehicleType::Helicopter:
            return baseThreat * 3.5f;
        case VehicleType::Jet:
            return baseThreat * 3.0f;
        case VehicleType::Jeep:
            return baseThreat * 2.0f;
        case VehicleType::Boat:
            return baseThreat * 2.0f;
        default:
            return baseThreat;
    }
}