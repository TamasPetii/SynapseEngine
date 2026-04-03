#include "EditorApiImpl.h"
#include "Engine/Scene/Scene.h"

namespace Syn {

    SceneSettings EditorApiImpl::GetSceneSettings() const {
        auto scene = _sceneManager->GetActiveScene();

        if (scene && scene->GetSettings()) {
            return *(scene->GetSettings());
        }

        return SceneSettings{};
    }

    void EditorApiImpl::SetSceneSettings(const SceneSettings& settings) {
        auto scene = _sceneManager->GetActiveScene();

        if (scene && scene->GetSettings()) {
            *(scene->GetSettings()) = settings;
        }
    }

}