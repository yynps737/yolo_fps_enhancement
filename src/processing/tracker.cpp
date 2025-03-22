#include "tracker.h"

Tracker::Tracker(int maxLostFrames)
    : m_nextId(0),
      m_maxLostFrames(maxLostFrames)
{
}

std::vector<Entity> Tracker::update(const std::vector<Detection>& detections) {
    std::vector<Entity> trackedEntities;
    std::vector<bool> detectionMatched(detections.size(), false);

    // 更新已跟踪的对象
    for (auto& [id, trackedObj] : m_trackedObjects) {
        bool matched = false;
        int bestMatch = -1;
        float bestIOU = 0.3f; // IOU阈值

        // 查找最佳匹配的检测结果
        for (size_t i = 0; i < detections.size(); ++i) {
            if (detectionMatched[i]) {
                continue;
            }

            float iou = calculateIOU(trackedObj.boundingBox, detections[i].boundingBox);

            if (iou > bestIOU) {
                bestIOU = iou;
                bestMatch = static_cast<int>(i);
                matched = true;
            }
        }

        if (matched) {
            // 更新跟踪对象
            const auto& detection = detections[bestMatch];

            // 计算位置变化
            cv::Point center(
                detection.boundingBox.x + detection.boundingBox.width / 2,
                detection.boundingBox.y + detection.boundingBox.height / 2
            );

            cv::Point prevCenter(
                trackedObj.boundingBox.x + trackedObj.boundingBox.width / 2,
                trackedObj.boundingBox.y + trackedObj.boundingBox.height / 2
            );

            trackedObj.velocity.x = center.x - prevCenter.x;
            trackedObj.velocity.y = center.y - prevCenter.y;

            trackedObj.boundingBox = detection.boundingBox;
            trackedObj.lostFrames = 0;

            // 更新实体数据
            trackedObj.entityData.boundingBox = detection.boundingBox;
            trackedObj.entityData.confidence = detection.confidence;
            trackedObj.entityData.className = detection.className;
            trackedObj.entityData.keypoints = detection.keypoints;

            detectionMatched[bestMatch] = true;
        } else {
            // 增加丢失帧计数
            trackedObj.lostFrames++;

            // 使用速度信息预测位置
            trackedObj.boundingBox.x += trackedObj.velocity.x;
            trackedObj.boundingBox.y += trackedObj.velocity.y;

            trackedObj.entityData.boundingBox = trackedObj.boundingBox;
        }

        // 如果对象仍在跟踪中，添加到结果中
        if (trackedObj.lostFrames < m_maxLostFrames) {
            trackedEntities.push_back(trackedObj.entityData);
        }
    }

    // 移除长时间丢失的对象
    for (auto it = m_trackedObjects.begin(); it != m_trackedObjects.end();) {
        if (it->second.lostFrames >= m_maxLostFrames) {
            it = m_trackedObjects.erase(it);
        } else {
            ++it;
        }
    }

    // 添加新检测到的对象
    for (size_t i = 0; i < detections.size(); ++i) {
        if (!detectionMatched[i]) {
            const auto& detection = detections[i];

            TrackedObject newObj;
            newObj.id = m_nextId++;
            newObj.boundingBox = detection.boundingBox;
            newObj.velocity = cv::Point(0, 0);
            newObj.lostFrames = 0;

            // 创建实体数据
            Entity entity;
            entity.id = newObj.id;
            entity.boundingBox = detection.boundingBox;
            entity.confidence = detection.confidence;
            entity.className = detection.className;
            entity.keypoints = detection.keypoints;

            // 设置默认值
            entity.type = EntityType::Unknown;
            entity.team = TeamType::Unknown;

            newObj.entityData = entity;

            m_trackedObjects[newObj.id] = newObj;
            trackedEntities.push_back(entity);
        }
    }

    return trackedEntities;
}

void Tracker::reset() {
    m_trackedObjects.clear();
    m_nextId = 0;
}

float Tracker::calculateIOU(const cv::Rect& box1, const cv::Rect& box2) const {
    int x1 = std::max(box1.x, box2.x);
    int y1 = std::max(box1.y, box2.y);
    int x2 = std::min(box1.x + box1.width, box2.x + box2.width);
    int y2 = std::min(box1.y + box1.height, box2.y + box2.height);

    if (x2 < x1 || y2 < y1) {
        return 0.0f;
    }

    float intersectionArea = (x2 - x1) * (y2 - y1);
    float box1Area = box1.width * box1.height;
    float box2Area = box2.width * box2.height;

    return intersectionArea / (box1Area + box2Area - intersectionArea);
}