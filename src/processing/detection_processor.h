#pragma once

#include <map>
#include <memory>
#include "../detection/yolo_detector.h"
#include "entity.h"
#include "tracker.h"

enum class DistanceModel {
    Linear,
    Inverse,
    Logarithmic,
    Custom
};

class DetectionProcessor {
public:
    DetectionProcessor();
    
    ProcessedGameData processDetections(const std::vector<Detection>& detections, 
                                      const GameContext& context);
    
    void setClassMapping(const std::map<int, EntityType>& classMapping);
    void enableTracking(bool enable);
    void setDistanceEstimationModel(DistanceModel model);
    
    void enableMotionPrediction(bool enable, int frameOffset = 5);
    
private:
    Entity createEntityFromDetection(const Detection& detection);
    float estimateDistance(const cv::Rect& box, const GameContext& context);
    Vec3 estimateWorldPosition(const Detection& detection, const GameContext& context);
    std::vector<TargetInfo> generateTargets(const std::vector<Entity>& entities, const GameContext& context);
    float calculateThreatLevel(const Entity& entity, const GameContext& context);
    
    std::map<int, EntityType> m_classMapping;
    
    std::unique_ptr<Tracker> m_tracker;
    bool m_trackingEnabled;
    
    DistanceModel m_distanceModel;
    
    bool m_predictionEnabled;
    int m_predictionFrames;
    
    std::map<TargetZone, Vec3> m_targetOffsets;
};