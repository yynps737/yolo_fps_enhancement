#include "cod_adapter.h"
#include "../utils/logger.h"

CODAdapter::CODAdapter() {
    m_gameInfo.name = "Call of Duty";
    m_gameInfo.modelName = "cod_model.onnx";
    m_gameInfo.aspectRatio = 16.0f / 9.0f;
    m_gameInfo.defaultFov = 80.0f;
    
    m_classMapping = {
        {0, EntityType::Player},  // 敌方玩家
        {1, EntityType::Player},  // 友方玩家
        {2, EntityType::Weapon},  // 步枪
        {3, EntityType::Weapon},  // 冲锋枪
        {4, EntityType::Weapon},  // 轻机枪
        {5, EntityType::Weapon},  // 狙击枪
        {6, EntityType::Weapon},  // 霰弹枪
        {7, EntityType::Weapon},  // 手枪
        {8, EntityType::Weapon},  // 发射器
        {9, EntityType::Item},    // 护甲板
        {10, EntityType::Item},   // 战术装备
        {11, EntityType::Item},   // 致命装备
        {12, EntityType::Item},   // 野战升级
        {13, EntityType::Item},   // 弹药箱
        {14, EntityType::Item},   // 医疗箱
        {15, EntityType::Item},   // 自助复活
        {16, EntityType::Player}, // 载具
        {17, EntityType::Player}, // 击杀链（无人机等）
        {18, EntityType::Player}  // AI敌人
    };
    
    m_targetOffsets = {
        {TargetZone::Head, {0.0f, -0.22f, 0.05f}},
        {TargetZone::Chest, {0.0f, -0.05f, 0.1f}},
        {TargetZone::Stomach, {0.0f, 0.05f, 0.1f}},
        {TargetZone::Limbs, {0.0f, 0.15f, 0.1f}}
    };
    
    setDistanceParams(2.0f, 10.0f, 0.85f);
    
    Logger::info("COD适配器初始化");
}

void CODAdapter::processGameSpecific(ProcessedGameData& gameData) {
    // 识别武器类型
    identifyWeaponClasses(gameData);
    
    // 处理角色特性
    processOperators(gameData);
    
    // 计算击杀链威胁
    calculateKillstreakThreats(gameData);
    
    // 应用Warzone特定处理
    bool isWZ = isWarzone();
    float distanceScale = adjustDistanceScale();
    
    for (auto& player : gameData.players) {
        // 计算距离
        float estHeight = static_cast<float>(player.boundingBox.height);
        float distance = m_referenceDistance * (m_referenceHeight / estHeight);
        player.estimatedDistance = distance * m_scaleFactor * distanceScale;
        
        // 计算威胁等级
        player.threatLevel = estimateThreatLevel(player, GameContext());
        
        // Warzone中处理护甲板
        if (isWZ) {
            float armorPlates = calculateArmorPlates(player);
            player.estimatedHealth += static_cast<int>(armorPlates * 50.0f);
        }
        
        // 处理武器类型特性
        CODWeaponClass weaponClass = classifyWeaponType(player);
        
        switch (weaponClass) {
            case CODWeaponClass::Sniper:
                player.weaponType = WeaponType::Sniper;
                player.threatLevel *= 1.5f;
                break;
            case CODWeaponClass::AssaultRifle:
                player.weaponType = WeaponType::Rifle;
                player.threatLevel *= 1.2f;
                break;
            case CODWeaponClass::Shotgun:
                player.weaponType = WeaponType::Shotgun;
                if (player.estimatedDistance < 10.0f) {
                    player.threatLevel *= 2.0f;
                } else {
                    player.threatLevel *= 0.7f;
                }
                break;
            case CODWeaponClass::SMG:
                player.weaponType = WeaponType::SMG;
                if (player.estimatedDistance < 20.0f) {
                    player.threatLevel *= 1.5f;
                } else {
                    player.threatLevel *= 0.9f;
                }
                break;
            default:
                break;
        }
    }
    
    // 更新游戏环境信息
    gameData.environment.mapName = detectCurrentMap(cv::Mat());
    gameData.environment.roundNumber = 0;
    gameData.environment.isMatchPoint = false;
    
    // 特定游戏状态
    gameData.gameState.isBombPlanted = detectBombPlanted(cv::Mat());
}

bool CODAdapter::detectGameMode(const cv::Mat& frame) {
    if (detectWarzoneMode(frame)) {
        Logger::info("检测到战争地带模式");
        return true;
    } else if (detectResurgenceMode(frame)) {
        Logger::info("检测到重生模式");
        return true;
    } else if (detectTDMMode(frame)) {
        Logger::info("检测到团队死斗模式");
        return true;
    } else if (detectDominationMode(frame)) {
        Logger::info("检测到控制点模式");
        return true;
    } else if (detectHardpointMode(frame)) {
        Logger::info("检测到据点攻防模式");
        return true;
    } else if (detectSearchAndDestroyMode(frame)) {
        Logger::info("检测到搜索与摧毁模式");
        return true;
    } else if (detectFFAMode(frame)) {
        Logger::info("检测到自由混战模式");
        return true;
    }
    
    return false;
}

bool CODAdapter::analyzeMapContext(const cv::Mat& frame) {
    std::string mapName = detectCurrentMap(frame);
    if (!mapName.empty()) {
        Logger::info("检测到地图：" + mapName);
    }
    
    int playerCount = detectPlayerCount(frame);
    if (playerCount > 0) {
        Logger::info("检测到玩家数量：" + std::to_string(playerCount));
    }
    
    bool bombPlanted = detectBombPlanted(frame);
    if (bombPlanted) {
        Logger::info("检测到炸弹已安装");
    }
    
    int killstreakStatus = detectKillstreakStatus(frame);
    if (killstreakStatus > 0) {
        Logger::info("检测到击杀链状态：" + std::to_string(killstreakStatus));
    }
    
    return true;
}

bool CODAdapter::detectTDMMode(const cv::Mat& frame) {
    // 实际项目中会实现复杂检测
    return false;
}

bool CODAdapter::detectDominationMode(const cv::Mat& frame) {
    return false;
}

bool CODAdapter::detectHardpointMode(const cv::Mat& frame) {
    return false;
}

bool CODAdapter::detectSearchAndDestroyMode(const cv::Mat& frame) {
    return false;
}

bool CODAdapter::detectFFAMode(const cv::Mat& frame) {
    return false;
}

bool CODAdapter::detectWarzoneMode(const cv::Mat& frame) {
    return false;
}

bool CODAdapter::detectResurgenceMode(const cv::Mat& frame) {
    return false;
}

std::string CODAdapter::detectCurrentMap(const cv::Mat& frame) {
    // 实际项目中会实现地图识别
    return "";
}

int CODAdapter::detectPlayerCount(const cv::Mat& frame) {
    // 实际项目中会使用OCR识别玩家数量
    return 0;
}

bool CODAdapter::detectBombPlanted(const cv::Mat& frame) {
    // 检测炸弹状态
    return false;
}

int CODAdapter::detectKillstreakStatus(const cv::Mat& frame) {
    // 检测击杀链状态
    return 0;
}

int CODAdapter::detectScore(const cv::Mat& frame) {
    // 检测比分
    return 0;
}

void CODAdapter::identifyWeaponClasses(ProcessedGameData& gameData) {
    for (auto& player : gameData.players) {
        CODWeaponClass weaponClass = classifyWeaponType(player);
        
        // 根据武器类型设置不同属性
        switch (weaponClass) {
            case CODWeaponClass::AssaultRifle:
                // 步枪通常全能型
                break;
            case CODWeaponClass::SMG:
                // 冲锋枪特点是近距离伤害高
                break;
            case CODWeaponClass::Sniper:
                // 狙击枪一击致命
                break;
            default:
                break;
        }
    }
}

void CODAdapter::processOperators(ProcessedGameData& gameData) {
    // 处理角色特性
    // 在COD中，不同角色有不同的特性和外观
}

void CODAdapter::calculateKillstreakThreats(ProcessedGameData& gameData) {
    // 计算击杀链威胁
    // 在COD中，无人机、空袭等击杀链是重要威胁
}

CODWeaponClass CODAdapter::classifyWeaponType(const Entity& entity) {
    // 根据类名和视觉特征识别武器类型
    if (entity.className.find("ar") != std::string::npos || 
        entity.className.find("assault") != std::string::npos) {
        return CODWeaponClass::AssaultRifle;
    } else if (entity.className.find("smg") != std::string::npos) {
        return CODWeaponClass::SMG;
    } else if (entity.className.find("sniper") != std::string::npos) {
        return CODWeaponClass::Sniper;
    } else if (entity.className.find("shotgun") != std::string::npos) {
        return CODWeaponClass::Shotgun;
    } else if (entity.className.find("lmg") != std::string::npos) {
        return CODWeaponClass::LMG;
    } else if (entity.className.find("marksman") != std::string::npos) {
        return CODWeaponClass::Marksman;
    } else if (entity.className.find("pistol") != std::string::npos) {
        return CODWeaponClass::Pistol;
    } else if (entity.className.find("launcher") != std::string::npos) {
        return CODWeaponClass::Launcher;
    } else if (entity.className.find("melee") != std::string::npos) {
        return CODWeaponClass::Melee;
    }
    
    return CODWeaponClass::Unknown;
}

float CODAdapter::calculateArmorPlates(const Entity& entity) {
    // 根据视觉特征估算护甲板数量
    // 在Warzone中，蓝色护甲图标表示装备的护甲板
    return 3.0f; // 默认满护甲
}

float CODAdapter::estimateThreatLevel(const Entity& entity, const GameContext& context) {
    float baseThreat = 10.0f / (entity.estimatedDistance + 1.0f);
    
    // 根据武器、距离、玩家行为等因素调整威胁等级
    if (entity.boundingBox.height < (entity.boundingBox.width * 1.5f)) {
        // 可能在蹲下状态
        baseThreat *= 1.2f;
    }
    
    if (entity.className.find("killstreak") != std::string::npos) {
        // 击杀链威胁更高
        baseThreat *= 2.0f;
    }
    
    return baseThreat;
}

bool CODAdapter::isWarzone() {
    // 检测当前是否为Warzone模式
    return false;
}

bool CODAdapter::isMultiplayer() {
    // 检测当前是否为多人对战模式
    return true;
}

float CODAdapter::adjustDistanceScale() {
    // 根据游戏模式调整距离缩放因子
    if (isWarzone()) {
        return 1.5f; // Warzone地图更大
    } else {
        return 1.0f;
    }
}