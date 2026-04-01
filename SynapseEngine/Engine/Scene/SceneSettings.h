#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    struct SYN_API SceneSettings
    {
        bool enableTransparentPicking = false;

        bool enableGpuCulling = true;
        bool enableBloom = false;
        bool enableHiz = true;
        bool enableOcclusionCulling = true;

		bool enablePointLightSphereWireframe = false;
		bool enablePointLightAabbWireframe = false;
		bool enableSpotLightSphereWireframe = false;
		bool enableSpotLightAabbWireframe = false;

        bool enableWireframeMeshAabb = false;
        bool enableWireframeMeshSphere = false;
        bool enableWireframeMeshletAabb = false;
        bool enableWireframeMeshletSphere = false;

		bool enableDeferredEmissiveAo = true;
		bool enableDeferredPointLights = true;
		bool enableDeferredSpotLights = true;
		bool enableDeferredDirectionalLights = true;
        
        //Todo transparent settings
    };
}