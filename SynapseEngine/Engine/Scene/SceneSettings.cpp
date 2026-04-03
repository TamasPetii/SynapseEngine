#include "SceneSettings.h"

namespace Syn
{
    SceneSettings::SceneSettings()
        : enableTransparentPicking(false)
        , enableGpuCulling(true)
        , enableBloom(true)
        , enableHiz(true)
        , enableOcclusionCulling(true)
        , enablePointLightSphereWireframe(false)
        , enablePointLightAabbWireframe(false)
        , enableSpotLightSphereWireframe(false)
        , enableSpotLightAabbWireframe(false)
        , enableWireframeMeshAabb(false)
        , enableWireframeMeshSphere(false)
        , enableWireframeMeshletAabb(false)
        , enableWireframeMeshletSphere(false)
        , enableDeferredEmissiveAo(true)
        , enableDeferredPointLights(true)
        , enableDeferredSpotLights(true)
        , enableDeferredDirectionalLights(true)
    {}
}