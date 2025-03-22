#include "tracker.h"
#include <algorithm>

Tracker::Tracker(int maxLostFrames)
    : m_nextId(0),
      m_maxLostFrames(maxLostFrames)
{
}

std::vector<Entity> Tracker::update(const std::vector<Detection>& detections) {
    std::vector<Entity> trackedEntities;

    if (detections.empty()) {
        // 如果没有检测到新目标，增加所有跟踪对象的丢失帧计数
        for (auto& [id, trackedObj] : m_trackedObjects) {
            trackedObj.lostFrames++;

            // 使用速度信息预测位置
            if (trackedObj.lostFrames < m_maxLostFrames) {
                trackedObj.boundingBox.x += trackedObj.velocity.x;
                trackedObj.boundingBox.y += trackedObj.velocity.y;

                trackedObj.entityData.boundingBox = trackedObj.boundingBox;
                trackedEntities.push_back(trackedObj.entityData);
            }
        }
    } else {
        std::vector<bool> detectionMatched(detections.size(), false);

        // 更新已跟踪的对象
        for (auto& [id, trackedObj] : m_trackedObjects) {
            trackedObj.lostFrames++;

            // 使用速度信息预测位置
            if (trackedObj.lostFrames < m_maxLostFrames) {
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

                    // 计算位置和尺寸变化
                    cv::Point center(
                        detection.boundingBox.x + detection.boundingBox.width / 2,
                        detection.boundingBox.y + detection.boundingBox.height / 2
                    );

                    cv::Point prevCenter(
                        trackedObj.boundingBox.x + trackedObj.boundingBox.width / 2,
                        trackedObj.boundingBox.y + trackedObj.boundingBox.height / 2
                    );

                    // 更新速度（使用滑动平均平滑速度变化）
                    float alpha = 0.7f;
                    int newVelocityX = center.x - prevCenter.x;
                    int newVelocityY = center.y - prevCenter.y;

                    trackedObj.velocity.x = static_cast<int>(alpha * newVelocityX + (1 - alpha) * trackedObj.velocity.x);
                    trackedObj.velocity.y = static_cast<int>(alpha * newVelocityY + (1 - alpha) * trackedObj.velocity.y);

                    // 更新边界框和状态
                    trackedObj.boundingBox = detection.boundingBox;
                    trackedObj.lostFrames = 0;

                    // 更新实体数据，保留ID
                    Entity entity = createEntityFromDetection(detection);
                    entity.id = trackedObj.id;
                    trackedObj.entityData = entity;

                    // 标记检测结果已匹配
                    detectionMatched[bestMatch] = true;

                    // 记录最佳匹配结果
                    detectionMatched[bestMatch] = true;
                } else {
                    // 增加丢失帧计数
                    trackedObj.lostFrames++;
                }

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
                Entity entity = createEntityFromDetection(detection);
                entity.id = newObj.id;
                newObj.entityData = entity;

                m_trackedObjects[newObj.id] = newObj;
                trackedEntities.push_back(entity);
            }
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

Entity Tracker::createEntityFromDetection(const Detection& detection) {
    Entity entity;
    entity.boundingBox = detection.boundingBox;
    entity.confidence = detection.confidence;
    entity.className = detection.className;
    entity.keypoints = detection.keypoints;

    // 类型判断逻辑（基于类名）
    if (detection.className.find("player") != std::string::npos ||
        detection.className.find("enemy") != std::string::npos ||
        detection.className.find("person") != std::string::npos) {
        entity.type = EntityType::Player;
    } else if (detection.className.find("weapon") != std::string::npos ||
               detection.className.find("gun") != std::string::npos) {
        entity.type = EntityType::Weapon;
    } else if (detection.className.find("item") != std::string::npos) {
        entity.type = EntityType::Item;
    } else {
        entity.type = EntityType::Unknown;
    }

    // 团队判断逻辑
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

    // 计算屏幕位置
    entity.screenPosition = {
        static_cast<float>(entity.boundingBox.x + entity.boundingBox.width / 2),
        static_cast<float>(entity.boundingBox.y + entity.boundingBox.height / 2)
    };

    // 默认生命值
    entity.estimatedHealth = 100;

    return entity;
}