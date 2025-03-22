#pragma once

#include <string>
#include <map>
#include <opencv2/opencv.hpp>
#include "../processing/entity.h"

struct GameInfo {
    std::string name;
    std::string modelName;
    float aspectRatio;
    float defaultFov;
};

class GameAdapter {
public:
    virtual ~GameAdapter() = default;

    virtual void processGameSpecific(ProcessedGameData& gameData) = 0;
    virtual bool detectGameMode(const cv::Mat& frame) = 0;
    virtual bool analyzeMapContext(const cv::Mat& frame) = 0;

    void setDistanceParams(float referenceHeight, float referenceDistance, float scaleFactor);
    GameInfo getGameInfo() const;
    std::map<int, EntityType> getClassMapping() const;
    std::map<TargetZone, Vec3> getTargetOffsets() const;

    void updateClassMapping(const std::map<int, EntityType>& classMapping);
    void updateTargetOffsets(const std::map<TargetZone, Vec3>& targetOffsets);

    float getReferenceHeight() const;
    float getReferenceDistance() const;
    float getScaleFactor() const;

protected:
    GameInfo m_gameInfo;
    std::map<int, EntityType> m_classMapping;
    std::map<TargetZone, Vec3> m_targetOffsets;

    float m_referenceHeight;
    float m_referenceDistance;
    float m_scaleFactor;
};