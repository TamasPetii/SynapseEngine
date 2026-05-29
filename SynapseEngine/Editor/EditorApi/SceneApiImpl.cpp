#include "EditorApiImpl.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {

    static std::filesystem::path GetSceneCacheDirectory()
    {
        const char* appDataPath = std::getenv("APPDATA");

        std::filesystem::path baseDir = appDataPath ? appDataPath : ".";
        std::filesystem::path saveDir = baseDir / "Synapse" / "Cache" / "Scenes";

        if (!std::filesystem::exists(saveDir))
            std::filesystem::create_directories(saveDir);

        return saveDir;
    }

    void EditorApiImpl::NewScene() {
        Syn::Info("EditorApiImpl: New Scene intent triggered.");
    }

    void EditorApiImpl::LoadScene(const std::string& filepath)
    {
        std::filesystem::path loadPath;

        if (filepath.empty())
        {
            auto cacheDir = GetSceneCacheDirectory();
            loadPath = cacheDir / "Temp.synscene";
        }
        else
        {
            loadPath = filepath;
        }

        _sceneManager->LoadSceneFromFile(loadPath.string());

        Syn::Info("EditorApiImpl: Scene loaded from {}", loadPath.string());
    }

    void EditorApiImpl::SaveScene(const std::string& filepath) {
        auto activeScene = _sceneManager->GetActiveScene();

        if (!activeScene)
            return;

        std::filesystem::path savePath;

        if (filepath.empty())
        {
            auto cacheDir = GetSceneCacheDirectory();
            savePath = cacheDir / "Temp.synscene";
        }
        else
        {
            savePath = filepath;
        }

        _sceneManager->SaveActiveScene(savePath.string());

        Syn::Info("EditorApiImpl: Scene saved to {}", savePath.string());
    }
}