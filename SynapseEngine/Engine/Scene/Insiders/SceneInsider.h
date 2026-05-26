#pragma once
#include "Engine/SynApi.h"
#include "Engine/Passkey.h"
#include "Engine/Registry/Entity.h"

namespace Syn
{
    class Scene;
    class Registry;
    struct SceneSettings;

    class SYN_API SceneInsider
    {
    public:
        static Passkey<SceneInsider> GetKey() { return {}; }

        static EntityID& GetSceneCameraEntity(Scene& scene, Passkey<SceneInsider>);
        static EntityID& GetDebugCameraEntity(Scene& scene, Passkey<SceneInsider>);
        static Registry& GetRegistry(Scene& scene, Passkey<SceneInsider>);
        static SceneSettings& GetSettings(Scene& scene, Passkey<SceneInsider>);
    };
}