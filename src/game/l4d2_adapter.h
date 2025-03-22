#pragma once

#include "game_adapter.h"

enum class ZombieType {
    Common,
    Boomer,
    Hunter,
    Smoker,
    Tank,
    Witch,
    Spitter,
    Jockey,
    Charger,
    Unknown
};

class L4D2Adapter : public GameAdapter {
public:
    L4D2Adapter();
    
    void processGameSpecific(ProcessedGameData& gameData) override;
    bool detectGameMode(const cv::Mat& frame) override;
    bool analyzeMapContext(const cv::Mat& frame) override;
    
private:
    bool detectCampaignMode(const cv::Mat& frame);
    bool detectVersusMode(const cv::Mat& frame);
    bool detectSurvivalMode(const cv::Mat& frame);
    bool detectScavengeMode(const cv::Mat& frame);
    
    std::string detectCurrentMap(const cv::Mat& frame);
    bool detectHordeEvent(const cv::Mat& frame);
    bool detectTankPresent(const cv::Mat& frame);
    
    void identifyZombieTypes(ProcessedGameData& gameData);
    void processSurvivorStatus(ProcessedGameData& gameData);
    void updateHordeInfo(ProcessedGameData& gameData, const cv::Mat& frame);
    
    ZombieType classifyZombieType(const Entity& entity);
    float calculateZombieThreat(const Entity& entity, ZombieType type);
};