#include "game_adapter.h"
#include "../utils/logger.h"

void GameAdapter::setDistanceParams(float referenceHeight, float referenceDistance, float scaleFactor) {
    m_referenceHeight = referenceHeight;
    m_referenceDistance = referenceDistance;
    m_scaleFactor = scaleFactor;

    Logger::debug("设置距离参数: 参考高度=" + std::to_string(referenceHeight) +
                ", 参考距离=" + std::to_string(referenceDistance) +
                ", 缩放因子=" + std::to_string(scaleFactor));
}

GameInfo GameAdapter::getGameInfo() const {
    return m_gameInfo;
}

std::map<int, EntityType> GameAdapter::getClassMapping() const {
    return m_classMapping;
}

std::map<TargetZone, Vec3> GameAdapter::getTargetOffsets() const {
    return m_targetOffsets;
}

void GameAdapter::updateClassMapping(const std::map<int, EntityType>& classMapping) {
    m_classMapping = classMapping;
    Logger::info("更新类别映射，共 " + std::to_string(m_classMapping.size()) + " 个类别");
}

void GameAdapter::updateTargetOffsets(const std::map<TargetZone, Vec3>& targetOffsets) {
    m_targetOffsets = targetOffsets;
    Logger::info("更新目标偏移量，共 " + std::to_string(m_targetOffsets.size()) + " 个区域");
}

float GameAdapter::getReferenceHeight() const {
    return m_referenceHeight;
}

float GameAdapter::getReferenceDistance() const {
    return m_referenceDistance;
}

float GameAdapter::getScaleFactor() const {
    return m_scaleFactor;
}