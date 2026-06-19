#include "SceneInsider.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Registry/Registry.h"
#include "Engine/Scene/Settings/SceneSettings.h"

namespace Syn
{
    EntityID& SceneInsider::GetSceneCameraEntity(Scene& scene, Passkey<SceneInsider>) {
        return scene._sceneCameraEntity;
    }

    EntityID& SceneInsider::GetDebugCameraEntity(Scene& scene, Passkey<SceneInsider>) {
        return scene._debugCameraEntity;
    }

    Registry& SceneInsider::GetRegistry(Scene& scene, Passkey<SceneInsider>) {
        return *scene._registry;
    }

    SceneSettings& SceneInsider::GetSettings(Scene& scene, Passkey<SceneInsider>) {
        return *scene._sceneSettings;
    }
}