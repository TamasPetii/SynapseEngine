#include "SceneApiImpl.h"
#include "Engine/Logger/SynLog.h"
#include <filesystem>

namespace Syn {
    static std::filesystem::path GetSceneCacheDirectory() {
        const char* appDataPath = std::getenv("APPDATA");
        std::filesystem::path baseDir = appDataPath ? appDataPath : ".";
        std::filesystem::path saveDir = baseDir / "Synapse" / "Cache" / "Scenes";
        if (!std::filesystem::exists(saveDir)) std::filesystem::create_directories(saveDir);
        return saveDir;
    }

    void SceneApiImpl::NewScene() {
        Syn::Info("SceneApiImpl: New Scene intent triggered.");
    }

    void SceneApiImpl::LoadScene(const std::string& filepath) {
        std::filesystem::path loadPath = filepath.empty() ? GetSceneCacheDirectory() / "Temp.synscene" : filepath;
        _sceneManager->LoadSceneFromFile(loadPath.string());
        Syn::Info("SceneApiImpl: Scene loaded from {}", loadPath.string());
    }

    void SceneApiImpl::SaveScene(const std::string& filepath) {
        if (!_sceneManager->GetActiveScene()) return;
        std::filesystem::path savePath = filepath.empty() ? GetSceneCacheDirectory() / "Temp.synscene" : filepath;
        _sceneManager->SaveActiveScene(savePath.string());
        Syn::Info("SceneApiImpl: Scene saved to {}", savePath.string());
    }
}