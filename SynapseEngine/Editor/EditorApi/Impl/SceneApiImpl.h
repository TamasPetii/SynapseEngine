#pragma once
#include "EditorCore/Api/ISceneApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class SceneApiImpl : public ISceneApi {
    public:
        SceneApiImpl(SceneManager* sm) : _sceneManager(sm) {}
        void NewScene() override;
        void LoadScene(const std::string& filepath = "") override;
        void SaveScene(const std::string& filepath = "") override;
    private:
        SceneManager* _sceneManager;
    };
}