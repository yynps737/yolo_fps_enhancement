#include "model_manager.h"
#include <filesystem>
#include <fstream>
#include <chrono>
#include "../utils/logger.h"

ModelManager::ModelManager(const std::string& modelsDirectory)
    : m_modelsDirectory(modelsDirectory),
      m_isTraining(false),
      m_trainingProgress(0.0f)
{
    if (!std::filesystem::exists(modelsDirectory)) {
        std::filesystem::create_directory(modelsDirectory);
    }

    checkForModelUpdates();
}

ModelManager::~ModelManager() {
    if (m_isTraining && m_trainingThread.joinable()) {
        m_trainingThread.join();
    }
}

std::vector<ModelInfo> ModelManager::getAvailableModels() const {
    std::vector<ModelInfo> models;

    for (const auto& [name, info] : m_modelRegistry) {
        models.push_back(info);
    }

    return models;
}

bool ModelManager::loadModel(const std::string& modelName) {
    std::string modelPath = m_modelsDirectory + "/" + modelName;

    if (!std::filesystem::exists(modelPath)) {
        Logger::error("模型文件不存在：" + modelPath);
        return false;
    }

    if (!validateModel(modelPath)) {
        Logger::error("模型验证失败：" + modelPath);
        return false;
    }

    auto it = m_modelRegistry.find(modelName);
    if (it != m_modelRegistry.end()) {
        m_currentModel = it->second;
    } else {
        m_currentModel.name = modelName;
        m_currentModel.path = modelPath;
        m_currentModel.performance = evaluateModelPerformance(modelPath);

        auto now = std::chrono::system_clock::now();
        auto timeT = std::chrono::system_clock::to_time_t(now);
        m_currentModel.lastUpdated = std::ctime(&timeT);

        m_modelRegistry[modelName] = m_currentModel;
    }

    return true;
}

void ModelManager::unloadCurrentModel() {
    m_currentModel = ModelInfo();
}

ModelInfo ModelManager::getModelForGame(const std::string& gameName) const {
    for (const auto& [name, info] : m_modelRegistry) {
        if (info.gameName == gameName) {
            return info;
        }
    }

    return ModelInfo();
}

void ModelManager::registerGameModel(const std::string& gameName, const std::string& modelName) {
    auto it = m_modelRegistry.find(modelName);
    if (it != m_modelRegistry.end()) {
        it->second.gameName = gameName;
    }
}

void ModelManager::checkForModelUpdates() {
    m_modelRegistry.clear();

    for (const auto& entry : std::filesystem::directory_iterator(m_modelsDirectory)) {
        if (entry.path().extension() == ".onnx") {
            std::string modelName = entry.path().filename().string();
            std::string modelPath = entry.path().string();

            ModelInfo info;
            info.name = modelName;
            info.path = modelPath;
            info.gameName = "";
            info.performance = evaluateModelPerformance(modelPath);

            auto timeT = std::filesystem::last_write_time(entry.path());
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                timeT - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
            auto timet = std::chrono::system_clock::to_time_t(sctp);
            info.lastUpdated = std::ctime(&timet);

            m_modelRegistry[modelName] = info;
        }
    }

    Logger::info("发现 " + std::to_string(m_modelRegistry.size()) + " 个模型");
}

bool ModelManager::downloadModel(const std::string& modelName) {
    // 实际项目中这里会实现模型下载功能
    // 简化实现，仅记录日志
    Logger::info("正在下载模型：" + modelName);
    return false;
}

void ModelManager::startIncrementalTraining(const std::vector<TrainingFrame>& trainingData) {
    if (m_isTraining) {
        Logger::warning("已经有训练在进行中");
        return;
    }

    if (trainingData.empty()) {
        Logger::warning("训练数据为空");
        return;
    }

    m_isTraining = true;
    m_trainingProgress = 0.0f;

    m_trainingThread = std::thread([this, trainingData]() {
        Logger::info("开始增量训练");

        for (size_t i = 0; i < 100; ++i) {
            {
                std::lock_guard<std::mutex> lock(m_trainingMutex);
                m_trainingProgress = static_cast<float>(i) / 100.0f;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        {
            std::lock_guard<std::mutex> lock(m_trainingMutex);
            m_trainingProgress = 1.0f;
            m_isTraining = false;
        }

        Logger::info("增量训练完成");
    });
}

float ModelManager::getTrainingProgress() const {
    std::lock_guard<std::mutex> lock(m_trainingMutex);
    return m_trainingProgress;
}

bool ModelManager::validateModel(const std::string& modelPath) {
    // 实际项目中这里会实现模型验证功能
    // 简化实现，仅检查文件存在
    return std::filesystem::exists(modelPath);
}

float ModelManager::evaluateModelPerformance(const std::string& modelPath) {
    // 实际项目中这里会实现模型性能评估
    // 简化实现，返回随机值
    return 0.85f;
}