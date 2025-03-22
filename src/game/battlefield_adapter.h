#pragma once

#include "game_adapter.h"

enum class VehicleType {
    Tank,
    Jeep,
    APC,
    Helicopter,
    Jet,
    Boat,
    Unknown
};

enum class SoldierClass {
    Assault,
    Engineer,
    Support,
    Recon,
    Unknown
};

class BattlefieldAdapter : public GameAdapter {
public:
    BattlefieldAdapter();
    
    void processGameSpecific(ProcessedGameData& gameData) override;
    bool detectGameMode(const cv::Mat& frame) override;
    bool analyzeMapContext(const cv::Mat& frame) override;
    
private:
    bool detectConquestMode(const cv::Mat& frame);
    bool detectRushMode(const cv::Mat& frame);
    bool detectTDMMode(const cv::Mat& frame);
    bool detectDominationMode(const cv::Mat& frame);
    
    std::string detectCurrentMap(const cv::Mat& frame);
    int detectTicketCount(const cv::Mat& frame);
    int detectFlagStatus(const cv::Mat& frame);
    bool detectObjectiveStatus(const cv::Mat& frame);
    
    void identifySoldierClasses(ProcessedGameData& gameData);
    void processVehicles(ProcessedGameData& gameData);
    void calculateObjectivePriorities(ProcessedGameData& gameData);
    
    SoldierClass classifySoldierType(const Entity& entity);
    VehicleType classifyVehicleType(const Entity& entity);
    
    float calculateDistanceScale(const cv::Mat& frame);
    float calculateSoldierThreat(const Entity& entity, SoldierClass type);
    float calculateVehicleThreat(const Entity& entity, VehicleType type);
};