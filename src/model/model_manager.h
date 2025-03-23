#pragma once

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>

struct ModelInfo {
    std::string name;
    std::string path;
    std::string gameName;
    float performance;
    std::string lastUpdated;
};

struct TrainingFrame {
    std::string imagePath;
    std::vector<std::vector<float>> annotations;
};

class ModelManager {
public:
    ModelManager(const std::string& modelsDirectory);
    ~ModelManager();

    std::vector<ModelInfo> getAvailableModels() const;
    bool loadModel(const std::string& modelName);
    void unloadCurrentModel();

    ModelInfo getModelForGame(const std::string& gameName) const;
    void registerGameModel(const std::string& gameName, const std::string& modelName);

    void checkForModelUpdates();
    bool downloadModel(const std::string& modelName);

    void startIncrementalTraining(const std::vector<TrainingFrame>& trainingData);
    float getTrainingProgress() const;

private:
    bool validateModel(const std::string& modelPath);
    float evaluateModelPerformance(const std::string& modelPath);

    std::string m_modelsDirectory;
    std::map<std::string, ModelInfo> m_modelRegistry;
    ModelInfo m_currentModel;

    bool m_isTraining;
    float m_trainingProgress;
    std::thread m_trainingThread;
    mutable std::mutex m_trainingMutex;
};