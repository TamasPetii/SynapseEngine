#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API SceneSettings
    {
        SceneSettings();

        bool useDebugCamera;
        bool enableTransparentPicking;

        bool enableGpuCulling;
        bool enableBloom;
        bool enableHiz;
        bool enableOcclusionCulling;

        bool enablePointLightSphereWireframe;
        bool enablePointLightAabbWireframe;
        bool enableSpotLightSphereWireframe;
        bool enableSpotLightAabbWireframe;

        bool enableWireframeMeshAabb;
        bool enableWireframeMeshSphere;
        bool enableWireframeMeshletAabb;
        bool enableWireframeMeshletSphere;

        bool enableDeferredEmissiveAo;
        bool enableDeferredPointLights;
        bool enableDeferredSpotLights;
        bool enableDeferredDirectionalLights;

		bool enableBillboardPointLights;
		bool enableBillboardSpotLights;
		bool enableBillboardDirectionalLights;
    };
}