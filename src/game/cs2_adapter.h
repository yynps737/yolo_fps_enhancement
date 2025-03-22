#pragma once

#include "game_adapter.h"

class CS2Adapter : public GameAdapter {
public:
    CS2Adapter();

    void processGameSpecific(ProcessedGameData& gameData) override;
    bool detectGameMode(const cv::Mat& frame) override;
    bool analyzeMapContext(const cv::Mat& frame) override;

private:
    bool detectCompetitiveMode(const cv::Mat& frame);
    bool detectCasualMode(const cv::Mat& frame);
    bool detectDeathMatchMode(const cv::Mat& frame);

    std::string detectCurrentMap(const cv::Mat& frame);

    bool isBombPlanted(const cv::Mat& frame);
    bool isDefusing(const cv::Mat& frame);
};