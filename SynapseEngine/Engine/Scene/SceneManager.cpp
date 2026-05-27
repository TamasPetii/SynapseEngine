#include "SceneManager.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/Scene/Writer/ManifestSceneWriter.h"
#include "Engine/Scene/Loader/ManifestSceneLoader.h"
#include "Engine/Scene/Source/File/FileSceneSource.h"
#include "Engine/FrameContext.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn
{
    SceneManager::SceneManager(std::unique_ptr<ISceneWriter> writer, std::unique_ptr<ISceneLoader> loader)
        : _writer(std::move(writer)), _loader(std::move(loader))
    {}

    void SceneManager::RegisterScene(const std::string& name, SceneFactory factory)
    {
        _sceneFactories[name] = factory;
    }

    bool SceneManager::SaveActiveScene(const std::filesystem::path& path)
    {
        if (!_activeScene)
        {
            Error("SceneManager: Cannot save scene. No active scene is running!");
            return false;
        }

        if (!_writer)
        {
            Error("SceneManager: Cannot save scene. No ISceneWriter was injected!");
            return false;
        }

        Info("SceneManager: Saving active scene to file: {}", path.string());
        return _writer->SaveScene(*_activeScene, path);
    }

    bool SceneManager::LoadSceneFromFile(const std::filesystem::path& path)
    {
        if (!_loader)
        {
            Error("SceneManager: Cannot load scene. No ISceneLoader was injected!");
            return false;
        }

        Info("SceneManager: Loading scene from file path: {}", path.string());

        auto source = std::make_unique<FileSceneSource>(path, _loader.get());
        uint32_t framesInFlight = ServiceLocator::GetFrameContext()->framesInFlight;

        _pendingScene = std::make_unique<Scene>(framesInFlight, std::move(source));
        _isSceneChangePending = true;

        Info("SceneManager: Scene loaded into memory and queued for activation.");
        return true;
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
            if (_activeScene)
            {
                vkDeviceWaitIdle(ServiceLocator::GetVkContext()->GetDevice()->Handle());
            }

            if (_pendingScene)
            {
                _activeScene = std::move(_pendingScene);
                Info("Successfully activated deferred scene from file.");
            }
            else if (!_pendingSceneName.empty() && _sceneFactories.contains(_pendingSceneName))
            {
                _activeScene = std::move(_sceneFactories[_pendingSceneName]());
                Info("Successfully loaded and activated scene factory: {}", _pendingSceneName);
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