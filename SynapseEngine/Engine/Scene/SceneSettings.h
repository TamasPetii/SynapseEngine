#pragma once
#include "Engine/SynApi.h"
#include <cstdint>

namespace Syn
{
	enum SYN_API PipelineType
	{
		Deferred,
		ForwardPlus
	};

    enum SYN_API DebugVisibilityMode
    {
        EntityId = 0,
        Pipeline = 1,
        LodLevel = 2,
        MeshIndex = 3,
        MeshletIndex = 4,
        TriangleIndex = 5,
        AllCombined = 6,
        MaterialType = 7,
        ForwardPlusSlices = 8,
        ForwardPlusLightCount = 9
    };

    struct SYN_API SceneSettings
    {
        SceneSettings();

		PipelineType pipelineType;
        uint32_t tileSize;

        bool useDebugCamera;

        bool enableGpuCulling;
        bool enableStaticBvhCulling;
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
		bool enableSpotLightConeWireframe;
        bool enableSpotLightPyramidWireframe;

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

        bool enableDebugVisibility;
        DebugVisibilityMode debugVisibilityMode;
    };
}