#pragma once
#include "Engine/SynApi.h"
#include "Engine/Scene/Scene.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

namespace Syn
{
    class SYN_API SceneManager
    {
    public:
        using SceneFactory = std::function<std::shared_ptr<Scene>()>;

        SceneManager() = default;
        ~SceneManager() = default;

        void RegisterScene(const std::string& name, SceneFactory factory);
        void LoadScene(const std::string& name);

        std::shared_ptr<Scene> GetActiveScene() const { return _activeScene; }

        void Update(float deltaTime, uint32_t frameIndex);
        void UpdateGPU(uint32_t frameIndex);
        void Finish();
    private:
        void ApplyPendingSceneChange();
    private:
        std::unordered_map<std::string, SceneFactory> _sceneFactories;
        std::shared_ptr<Scene> _activeScene = nullptr;

        bool _isSceneChangePending = false;
        std::string _pendingSceneName = "";
    };
}