#include "SettingsApiImpl.h"
#include "Engine/Scene/Scene.h"

namespace Syn {
    SceneSettings SettingsApiImpl::GetSceneSettings() const {
        auto scene = _sceneManager->GetActiveScene();
        return (scene && scene->GetSettings()) ? *(scene->GetSettings()) : SceneSettings{};
    }

    void SettingsApiImpl::SetSceneSettings(const SceneSettings& settings) {
        auto scene = _sceneManager->GetActiveScene();
        if (scene && scene->GetSettings()) {
            *(scene->GetSettings()) = settings;
        }
    }
}