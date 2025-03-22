#pragma once

#include "game_adapter.h"

enum class CODGameMode {
    TDM,
    Domination,
    Hardpoint,
    SearchAndDestroy,
    FFA,
    Warzone,
    Resurgence,
    Unknown
};

enum class CODWeaponClass {
    AssaultRifle,
    SMG,
    LMG,
    Marksman,
    Sniper,
    Shotgun,
    Pistol,
    Launcher,
    Melee,
    Tactical,
    Lethal,
    Unknown
};

class CODAdapter : public GameAdapter {
public:
    CODAdapter();
    
    void processGameSpecific(ProcessedGameData& gameData) override;
    bool detectGameMode(const cv::Mat& frame) override;
    bool analyzeMapContext(const cv::Mat& frame) override;
    
private:
    bool detectTDMMode(const cv::Mat& frame);
    bool detectDominationMode(const cv::Mat& frame);
    bool detectHardpointMode(const cv::Mat& frame);
    bool detectSearchAndDestroyMode(const cv::Mat& frame);
    bool detectFFAMode(const cv::Mat& frame);
    bool detectWarzoneMode(const cv::Mat& frame);
    bool detectResurgenceMode(const cv::Mat& frame);
    
    std::string detectCurrentMap(const cv::Mat& frame);
    int detectPlayerCount(const cv::Mat& frame);
    bool detectBombPlanted(const cv::Mat& frame);
    int detectKillstreakStatus(const cv::Mat& frame);
    int detectScore(const cv::Mat& frame);
    
    void identifyWeaponClasses(ProcessedGameData& gameData);
    void processOperators(ProcessedGameData& gameData);
    void calculateKillstreakThreats(ProcessedGameData& gameData);
    
    CODWeaponClass classifyWeaponType(const Entity& entity);
    float calculateArmorPlates(const Entity& entity);
    float estimateThreatLevel(const Entity& entity, const GameContext& context);
    
    bool isWarzone();
    bool isMultiplayer();
    float adjustDistanceScale();
};