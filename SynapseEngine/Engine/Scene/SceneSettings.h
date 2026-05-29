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

        bool enableGeometryGpuCulling;
        bool enablePointLightGpuCulling;
        bool enableSpotLightGpuCulling;

        bool enableHiz;
        bool enableStaticBvhCulling;
        bool enableMortonBvhCulling;
        bool enableBloom;

        bool enableMeshletConeCulling;

        bool enableFrustumCulling;
        bool enableChunkFrustumCulling;
        bool enableModelFrustumCulling;
        bool enableMeshFrustumCulling;
        bool enableMeshletFrustumCulling;
        bool enablePointLightFrustumCulling;
        bool enableSpotLightFrustumCulling;

        bool enableOcclusionCulling;
        bool enableChunkOcclusionCulling;
        bool enableModelOcclusionCulling;
        bool enableMeshOcclusionCulling;
        bool enableMeshletOcclusionCulling;
        bool enablePointLightOcclusionCulling;
        bool enableSpotLightOcclusionCulling;

        bool enableDeferredEmissiveAo;
        bool enableDeferredPointLights;
        bool enableDeferredSpotLights;
        bool enableDeferredDirectionalLights;

        bool enableForwardPlusEmissiveAo;
        bool enableForwardPlusPointLights;
        bool enableForwardPlusSpotLights;
        bool enableForwardPlusDirectionalLights;

        bool enableWireframeMeshAabb;
        bool enableWireframeMeshSphere;

        bool enableWireframeMeshletAabb;
        bool enableWireframeMeshletSphere;
        bool enableWireframeMeshletCone;

        bool enableMortonChunkAabbWireframe;
        bool enableStaticChunkAabbWireframe;
        bool enablePointLightSphereWireframe;
        bool enablePointLightAabbWireframe;
        bool enableSpotLightSphereWireframe;
        bool enableSpotLightAabbWireframe;
		bool enableSpotLightConeWireframe;
        bool enableSpotLightPyramidWireframe;
        bool enableBoxColliderWireframe;
        bool enableSphereColliderWireframe;
        bool enableCapsuleColliderWireframe;

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

        float aoRadius = 0.930f;
        float aoIntensity = 100.0f;
        float maxOcclusionDistance = 3.0f;
        float depthSharpness = 0.0f;
        float bias = 0.005f;
        int sampleCount = 16;

		bool enableSsao;
        bool enableSsaoLight;
    };
}