#pragma once
#include "EditorCore/Api/ISettingsApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class SettingsApiImpl : public ISettingsApi {
    public:
        SettingsApiImpl(SceneManager* sm) : _sceneManager(sm) {}
        SceneSettings GetSceneSettings() const override;
        void SetSceneSettings(const SceneSettings& settings) override;
    private:
        SceneManager* _sceneManager;
    };
}