#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

enum class EntityType {
    Player,
    Weapon,
    Utility,
    Item,
    Projectile,
    Unknown
};

enum class TeamType {
    T,
    CT,
    Allied,
    Enemy,
    Neutral,
    Unknown
};

enum class WeaponType {
    Rifle,
    Pistol,
    Sniper,
    Shotgun,
    SMG,
    LMG,
    Melee,
    Utility,
    Unknown
};

enum class UtilityType {
    Grenade,
    Flashbang,
    Smoke,
    Molotov,
    C4,
    Unknown
};

enum class TargetZone {
    Head,
    Chest,
    Stomach,
    Limbs,
    Unknown
};

struct Vec2 {
    float x;
    float y;
};

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Entity {
    int id;
    EntityType type;
    TeamType team;
    cv::Rect boundingBox;
    float confidence;
    std::string className;
    std::vector<cv::Point> keypoints;
    
    WeaponType weaponType = WeaponType::Unknown;
    float estimatedDistance = 0.0f;
    float threatLevel = 0.0f;
    int estimatedHealth = 100;
    bool hasHelmet = false;
    bool isVisible = true;
    Vec3 worldPosition = {0.0f, 0.0f, 0.0f};
    Vec2 screenPosition = {0.0f, 0.0f};
};

struct TargetInfo {
    int entityId;
    TargetZone zone;
    cv::Point targetPoint;
    float priority;
    float distance;
    Vec2 aimVector;
};

struct GameContext {
    float fieldOfView;
    cv::Size screenSize;
    float aspectRatio;
    bool isAiming;
    std::string currentWeapon;
};

struct GameEnvironmentInfo {
    std::string mapName;
    int roundNumber;
    bool isMatchPoint;
};

struct GameStateInfo {
    int allyCount;
    int enemyCount;
    bool isBombPlanted;
    bool isDefusing;
};

struct ProcessedGameData {
    std::vector<Entity> players;
    std::vector<Entity> items;
    std::vector<Entity> projectiles;
    std::vector<TargetInfo> potentialTargets;
    
    GameEnvironmentInfo environment;
    GameStateInfo gameState;
};