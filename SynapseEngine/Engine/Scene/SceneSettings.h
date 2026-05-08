#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
	enum SYN_API PipelineType
	{
		Deferred,
		ForwardPlus
	};

    struct SYN_API SceneSettings
    {
        SceneSettings();

		PipelineType pipelineType;

        bool useDebugCamera;

        bool enableGpuCulling;
        bool enableBloom;
        bool enableHiz;
        bool enableOcclusionCulling;
        bool enableConeCulling;
        bool enableFrustumCulling;

        bool enableDeferredEmissiveAo;
        bool enableDeferredPointLights;
        bool enableDeferredSpotLights;
        bool enableDeferredDirectionalLights;

        bool enableWireframeMeshAabb;
        bool enableWireframeMeshSphere;

        bool enablePointLightSphereWireframe;
        bool enablePointLightAabbWireframe;
        bool enableSpotLightSphereWireframe;
        bool enableSpotLightAabbWireframe;

        bool enableBillboardCameras;
		bool enableBillboardPointLights;
		bool enableBillboardSpotLights;
        bool enableBillboardDirectionalLights;

        float ambientStrength;
        float emissiveStrength;

		float bloomThreshold;
		float bloomKnee;
        float bloomFilterRadius;
        float bloomExposure;
        float bloomStrength;
    };
}