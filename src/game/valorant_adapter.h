#pragma once

#include "game_adapter.h"

class ValorantAdapter : public GameAdapter {
public:
    ValorantAdapter();

    void processGameSpecific(ProcessedGameData& gameData) override;
    bool detectGameMode(const cv::Mat& frame) override;
    bool analyzeMapContext(const cv::Mat& frame) override;

private:
    bool detectCompetitiveMode(const cv::Mat& frame);
    bool detectUnratedMode(const cv::Mat& frame);
    bool detectDeathMatchMode(const cv::Mat& frame);

    std::string detectCurrentMap(const cv::Mat& frame);
    bool detectSpikePlanted(const cv::Mat& frame);
    bool detectDefusing(const cv::Mat& frame);
    int detectRoundNumber(const cv::Mat& frame);

    void identifyAgentTypes(ProcessedGameData& gameData);
    void processSpikeStatus(ProcessedGameData& gameData, const cv::Mat& frame);
};