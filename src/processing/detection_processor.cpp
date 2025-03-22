#include "detection_processor.h"

DetectionProcessor::DetectionProcessor()
    : m_trackingEnabled(true),
      m_distanceModel(DistanceModel::Inverse),
      m_predictionEnabled(false),
      m_predictionFrames(5)
{
    m_tracker = std::make_unique<Tracker>(30);
    
    m_classMapping = {
        {0, EntityType::Player},
        {1, EntityType::Player},
        {2, EntityType::Weapon},
        {3, EntityType::Weapon},
        {4, EntityType::Weapon},
        {5, EntityType::Utility}
    };
    
    m_targetOffsets = {
        {TargetZone::Head, {0.0f, -0.2f, 0.05f}},
        {TargetZone::Chest, {0.0f, -0.05f, 0.1f}},
        {TargetZone::Stomach, {0.0f, 0.05f, 0.1f}},
        {TargetZone::Limbs, {0.0f, 0.15f, 0.1f}}
    };
}

ProcessedGameData DetectionProcessor::processDetections(const std::vector<Detection>& detections, 
                                                    const GameContext& context) {
    ProcessedGameData gameData;
    std::vector<Entity> entities;
    
    if (m_trackingEnabled) {
        entities = m_tracker->update(detections);
    } else {
        for (const auto& detection : detections) {
            entities.push_back(createEntityFromDetection(detection));
        }
    }
    
    for (auto& entity : entities) {
        if (entity.type == EntityType::Player) {
            entity.estimatedDistance = estimateDistance(entity.boundingBox, context);
            entity.worldPosition = estimateWorldPosition(entity, context);
            entity.threatLevel = calculateThreatLevel(entity, context);
            
            gameData.players.push_back(entity);
        } else if (entity.type == EntityType::Weapon || entity.type == EntityType::Item) {
            gameData.items.push_back(entity);
        } else if (entity.type == EntityType::Projectile) {
            gameData.projectiles.push_back(entity);
        }
    }
    
    gameData.potentialTargets = generateTargets(gameData.players, context);
    
    gameData.gameState.allyCount = 0;
    gameData.gameState.enemyCount = 0;
    
    for (const auto& player : gameData.players) {
        if (player.team == TeamType::Allied || player.team == TeamType::CT) {
            gameData.gameState.allyCount++;
        } else if (player.team == TeamType::Enemy || player.team == TeamType::T) {
            gameData.gameState.enemyCount++;
        }
    }
    
    return gameData;
}

void DetectionProcessor::setClassMapping(const std::map<int, EntityType>& classMapping) {
    m_classMapping = classMapping;
}

void DetectionProcessor::enableTracking(bool enable) {
    m_trackingEnabled = enable;
    if (enable && !m_tracker) {
        m_tracker = std::make_unique<Tracker>(30);
    }
}

void DetectionProcessor::setDistanceEstimationModel(DistanceModel model) {
    m_distanceModel = model;
}

void DetectionProcessor::enableMotionPrediction(bool enable, int frameOffset) {
    m_predictionEnabled = enable;
    m_predictionFrames = frameOffset;
}

Entity DetectionProcessor::createEntityFromDetection(const Detection& detection) {
    Entity entity;
    
    entity.boundingBox = detection.boundingBox;
    entity.confidence = detection.confidence;
    entity.className = detection.className;
    entity.keypoints = detection.keypoints;
    
    auto it = m_classMapping.find(detection.classId);
    if (it != m_classMapping.end()) {
        entity.type = it->second;
    } else {
        entity.type = EntityType::Unknown;
    }
    
    if (entity.className.find("t_") == 0 || entity.className.find("terrorist") != std::string::npos) {
        entity.team = TeamType::T;
    } else if (entity.className.find("ct_") == 0 || entity.className.find("counter") != std::string::npos) {
        entity.team = TeamType::CT;
    } else if (entity.className.find("ally") != std::string::npos) {
        entity.team = TeamType::Allied;
    } else if (entity.className.find("enemy") != std::string::npos) {
        entity.team = TeamType::Enemy;
    } else {
        entity.team = TeamType::Unknown;
    }
    
    if (entity.className.find("rifle") != std::string::npos) {
        entity.weaponType = WeaponType::Rifle;
    } else if (entity.className.find("pistol") != std::string::npos) {
        entity.weaponType = WeaponType::Pistol;
    } else if (entity.className.find("sniper") != std::string::npos) {
        entity.weaponType = WeaponType::Sniper;
    } else if (entity.className.find("shotgun") != std::string::npos) {
        entity.weaponType = WeaponType::Shotgun;
    } else {
        entity.weaponType = WeaponType::Unknown;
    }
    
    entity.screenPosition = {
        static_cast<float>(entity.boundingBox.x + entity.boundingBox.width / 2),
        static_cast<float>(entity.boundingBox.y + entity.boundingBox.height / 2)
    };
    
    return entity;
}

float DetectionProcessor::estimateDistance(const cv::Rect& box, const GameContext& context) {
    const float referenceHeight = 300.0f;
    const float referenceDistance = 10.0f;
    
    float currentHeight = static_cast<float>(box.height);
    
    switch (m_distanceModel) {
        case DistanceModel::Linear:
            return (referenceHeight / currentHeight) * referenceDistance;
        
        case DistanceModel::Inverse:
            return referenceDistance * std::pow(referenceHeight / currentHeight, 1.2f);
        
        case DistanceModel::Logarithmic:
            return referenceDistance * std::log(referenceHeight / currentHeight + 1.0f) * 2.0f;
        
        case DistanceModel::Custom:
            return referenceDistance * (std::pow(referenceHeight / currentHeight, 1.5f) + 
                                     0.2f * std::log(referenceHeight / currentHeight + 1.0f));
        
        default:
            return referenceDistance * (referenceHeight / currentHeight);
    }
}

Vec3 DetectionProcessor::estimateWorldPosition(const Detection& detection, const GameContext& context) {
    Vec3 position;
    
    float centerX = detection.boundingBox.x + detection.boundingBox.width / 2.0f;
    float centerY = detection.boundingBox.y + detection.boundingBox.height / 2.0f;
    
    float normalizedX = (centerX / context.screenSize.width - 0.5f) * 2.0f;
    float normalizedY = (centerY / context.screenSize.height - 0.5f) * 2.0f;
    
    float distance = estimateDistance(detection.boundingBox, context);
    
    float fieldOfViewRadians = context.fieldOfView * (M_PI / 180.0f);
    float tanFov = std::tan(fieldOfViewRadians / 2.0f);
    
    position.x = normalizedX * distance * tanFov * context.aspectRatio;
    position.y = normalizedY * distance * tanFov;
    position.z = distance;
    
    return position;
}

std::vector<TargetInfo> DetectionProcessor::generateTargets(const std::vector<Entity>& entities, 
                                                          const GameContext& context) {
    std::vector<TargetInfo> targets;
    
    for (const auto& entity : entities) {
        if (entity.team == TeamType::Enemy || entity.team == TeamType::T) {
            TargetInfo headTarget;
            headTarget.entityId = entity.id;
            headTarget.zone = TargetZone::Head;
            headTarget.distance = entity.estimatedDistance;
            
            int offsetY = static_cast<int>(entity.boundingBox.height * m_targetOffsets[TargetZone::Head].y);
            headTarget.targetPoint = cv::Point(
                entity.boundingBox.x + entity.boundingBox.width / 2,
                entity.boundingBox.y + offsetY
            );
            
            headTarget.priority = 1.0f / (entity.estimatedDistance + 1.0f) * 10.0f;
            if (entity.weaponType == WeaponType::Sniper) {
                headTarget.priority *= 1.5f;
            }
            
            Vec2 screenCenter = {
                static_cast<float>(context.screenSize.width) / 2.0f,
                static_cast<float>(context.screenSize.height) / 2.0f
            };
            
            headTarget.aimVector = {
                headTarget.targetPoint.x - screenCenter.x,
                headTarget.targetPoint.y - screenCenter.y
            };
            
            targets.push_back(headTarget);
            
            TargetInfo bodyTarget;
            bodyTarget.entityId = entity.id;
            bodyTarget.zone = TargetZone::Chest;
            bodyTarget.distance = entity.estimatedDistance;
            
            offsetY = static_cast<int>(entity.boundingBox.height * m_targetOffsets[TargetZone::Chest].y);
            bodyTarget.targetPoint = cv::Point(
                entity.boundingBox.x + entity.boundingBox.width / 2,
                entity.boundingBox.y + entity.boundingBox.height / 3 + offsetY
            );
            
            bodyTarget.priority = 0.8f / (entity.estimatedDistance + 1.0f) * 10.0f;
            
            bodyTarget.aimVector = {
                bodyTarget.targetPoint.x - screenCenter.x,
                bodyTarget.targetPoint.y - screenCenter.y
            };
            
            targets.push_back(bodyTarget);
        }
    }
    
    std::sort(targets.begin(), targets.end(), [](const TargetInfo& a, const TargetInfo& b) {
        return a.priority > b.priority;
    });
    
    return targets;
}

float DetectionProcessor::calculateThreatLevel(const Entity& entity, const GameContext& context) {
    float threat = 10.0f / (entity.estimatedDistance + 1.0f);
    
    if (entity.weaponType == WeaponType::Sniper) {
        threat *= 1.5f;
    } else if (entity.weaponType == WeaponType::Rifle) {
        threat *= 1.2f;
    }
    
    Vec2 screenCenter = {
        static_cast<float>(context.screenSize.width) / 2.0f,
        static_cast<float>(context.screenSize.height) / 2.0f
    };
    
    float dx = entity.screenPosition.x - screenCenter.x;
    float dy = entity.screenPosition.y - screenCenter.y;
    float distanceToCenter = std::sqrt(dx * dx + dy * dy);
    
    threat *= (1.0f - std::min(1.0f, distanceToCenter / (context.screenSize.width / 2.0f)) * 0.5f);
    
    return threat;
}