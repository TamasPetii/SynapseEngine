#pragma once
#include "Engine/SynApi.h"
#include "Engine/Scene/Scene.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <filesystem>

#include "Engine/Scene/Writer/ISceneWriter.h"
#include "Engine/Scene/Loader/ISceneLoader.h"

namespace Syn
{
    class SYN_API SceneManager
    {
    public:
        using SceneFactory = std::function<std::unique_ptr<Scene>()>;

        SceneManager(std::unique_ptr<ISceneWriter> writer, std::unique_ptr<ISceneLoader> loader);
        ~SceneManager() = default;

        bool SaveActiveScene(const std::filesystem::path& path);
        bool LoadSceneFromFile(const std::filesystem::path& path);

        void RegisterScene(const std::string& name, SceneFactory factory);
        void LoadScene(const std::string& name);

        Scene* GetActiveScene() const { return _activeScene.get(); }

        void Update(float deltaTime, uint32_t frameIndex);
        void UpdateGPU(uint32_t frameIndex);
        void Finish();
    private:
        void ApplyPendingSceneChange();
    private:
        std::unique_ptr<ISceneWriter> _writer;
        std::unique_ptr<ISceneLoader> _loader;

        std::unordered_map<std::string, SceneFactory> _sceneFactories;
        std::unique_ptr<Scene> _activeScene = nullptr;
        std::unique_ptr<Scene> _pendingScene = nullptr;

        bool _isSceneChangePending = false;
        std::string _pendingSceneName = "";
    };
}