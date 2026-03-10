#include "SceneManager.h"
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    void SceneManager::RegisterScene(const std::string& name, SceneFactory factory)
    {
        _sceneFactories[name] = factory;
    }

    void SceneManager::LoadScene(const std::string& name)
    {
        if (_sceneFactories.contains(name))
        {
            _pendingSceneName = name;
            _isSceneChangePending = true;
            Info("Scene change requested: {}", name);
        }
        else
        {
            Error("Failed to load scene! Scene '{}' is not registered.", name);
        }
    }

    void SceneManager::ApplyPendingSceneChange()
    {
        if (_isSceneChangePending)
        {
            if (_sceneFactories.contains(_pendingSceneName))
            {
                _activeScene = _sceneFactories[_pendingSceneName]();
                Info("Successfully loaded scene: {}", _pendingSceneName);
            }

            _isSceneChangePending = false;
            _pendingSceneName = "";
        }
    }

    void SceneManager::Update(float deltaTime, uint32_t frameIndex)
    {
        if (_activeScene)
        {
            _activeScene->Update(deltaTime, frameIndex);
        }
    }

    void SceneManager::UpdateGPU(uint32_t frameIndex)
    {
        if (_activeScene)
        {
            _activeScene->UpdateGPU(frameIndex);
        }
    }

    void SceneManager::Finish()
    {
        if (_activeScene)
        {
            _activeScene->Finish();
        }

        ApplyPendingSceneChange();
    }
}