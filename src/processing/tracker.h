#pragma once

#include <vector>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include "entity.h"
#include "../detection/yolo_detector.h"

struct TrackedObject {
    int id;
    cv::Rect boundingBox;
    cv::Point velocity;
    int lostFrames;
    Entity entityData;
};

class Tracker {
public:
    Tracker(int maxLostFrames = 30);

    std::vector<Entity> update(const std::vector<Detection>& detections);
    void reset();

private:
    float calculateIOU(const cv::Rect& box1, const cv::Rect& box2) const;
    Entity createEntityFromDetection(const Detection& detection);
    
    std::unordered_map<int, TrackedObject> m_trackedObjects;
    int m_nextId;
    int m_maxLostFrames;
};