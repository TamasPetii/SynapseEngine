#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Scene/SceneSettings.h"
#include <type_traits>

namespace Syn
{
    template <>
    struct Schema<SceneSettings>
    {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& settings = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("pipelineType", reinterpret_cast<std::underlying_type_t<PipelineType>&>(settings.pipelineType));
            ar.Property("tileSize", settings.tileSize);
            ar.Property("useDebugCamera", settings.useDebugCamera);

            // Gpu Culling Toggles
            ar.Property("enableGeometryGpuCulling", settings.enableGeometryGpuCulling);
            ar.Property("enablePointLightGpuCulling", settings.enablePointLightGpuCulling);
            ar.Property("enableSpotLightGpuCulling", settings.enableSpotLightGpuCulling);

            // BVH, Hiz, Bloom
            ar.Property("enableHiz", settings.enableHiz);
            ar.Property("enableStaticBvhCulling", settings.enableStaticBvhCulling);
            ar.Property("enableMortonBvhCulling", settings.enableMortonBvhCulling);
            ar.Property("enableBloom", settings.enableBloom);
            ar.Property("enableMeshletConeCulling", settings.enableMeshletConeCulling);

            // Frustum Culling Toggles
            ar.Property("enableFrustumCulling", settings.enableFrustumCulling);
            ar.Property("enableChunkFrustumCulling", settings.enableChunkFrustumCulling);
            ar.Property("enableModelFrustumCulling", settings.enableModelFrustumCulling);
            ar.Property("enableMeshFrustumCulling", settings.enableMeshFrustumCulling);
            ar.Property("enableMeshletFrustumCulling", settings.enableMeshletFrustumCulling);
            ar.Property("enablePointLightFrustumCulling", settings.enablePointLightFrustumCulling);
            ar.Property("enableSpotLightFrustumCulling", settings.enableSpotLightFrustumCulling);

            // Occlusion Culling Toggles
            ar.Property("enableOcclusionCulling", settings.enableOcclusionCulling);
            ar.Property("enableChunkOcclusionCulling", settings.enableChunkOcclusionCulling);
            ar.Property("enableModelOcclusionCulling", settings.enableModelOcclusionCulling);
            ar.Property("enableMeshOcclusionCulling", settings.enableMeshOcclusionCulling);
            ar.Property("enableMeshletOcclusionCulling", settings.enableMeshletOcclusionCulling);
            ar.Property("enablePointLightOcclusionCulling", settings.enablePointLightOcclusionCulling);
            ar.Property("enableSpotLightOcclusionCulling", settings.enableSpotLightOcclusionCulling);

            // Deferred Passes Toggles
            ar.Property("enableDeferredEmissiveAo", settings.enableDeferredEmissiveAo);
            ar.Property("enableDeferredPointLights", settings.enableDeferredPointLights);
            ar.Property("enableDeferredSpotLights", settings.enableDeferredSpotLights);
            ar.Property("enableDeferredDirectionalLights", settings.enableDeferredDirectionalLights);

            // Forward Plus Passes Toggles
            ar.Property("enableForwardPlusEmissiveAo", settings.enableForwardPlusEmissiveAo);
            ar.Property("enableForwardPlusPointLights", settings.enableForwardPlusPointLights);
            ar.Property("enableForwardPlusSpotLights", settings.enableForwardPlusSpotLights);
            ar.Property("enableForwardPlusDirectionalLights", settings.enableForwardPlusDirectionalLights);

            // Wireframe Debug Toggles
            ar.Property("enableWireframeMeshAabb", settings.enableWireframeMeshAabb);
            ar.Property("enableWireframeMeshSphere", settings.enableWireframeMeshSphere);

            ar.Property("enableWireframeMeshletAabb", settings.enableWireframeMeshletAabb);
            ar.Property("enableWireframeMeshletSphere", settings.enableWireframeMeshletSphere);
            ar.Property("enableWireframeMeshletCone", settings.enableWireframeMeshletCone);

            ar.Property("enableMortonChunkAabbWireframe", settings.enableMortonChunkAabbWireframe);
            ar.Property("enableStaticChunkAabbWireframe", settings.enableStaticChunkAabbWireframe);
            ar.Property("enablePointLightSphereWireframe", settings.enablePointLightSphereWireframe);
            ar.Property("enablePointLightAabbWireframe", settings.enablePointLightAabbWireframe);
            ar.Property("enableSpotLightSphereWireframe", settings.enableSpotLightSphereWireframe);
            ar.Property("enableSpotLightAabbWireframe", settings.enableSpotLightAabbWireframe);
            ar.Property("enableSpotLightConeWireframe", settings.enableSpotLightConeWireframe);
            ar.Property("enableSpotLightPyramidWireframe", settings.enableSpotLightPyramidWireframe);
            ar.Property("enableBoxColliderWireframe", settings.enableBoxColliderWireframe);
            ar.Property("enableSphereColliderWireframe", settings.enableSphereColliderWireframe);
            ar.Property("enableCapsuleColliderWireframe", settings.enableCapsuleColliderWireframe);

            // Billboard Toggles
            ar.Property("enableBillboardCameras", settings.enableBillboardCameras);
            ar.Property("enableBillboardPointLights", settings.enableBillboardPointLights);
            ar.Property("enableBillboardSpotLights", settings.enableBillboardSpotLights);
            ar.Property("enableBillboardDirectionalLights", settings.enableBillboardDirectionalLights);

            // Material & Light Strengths
            ar.Property("ambientStrength", settings.ambientStrength);
            ar.Property("emissiveStrength", settings.emissiveStrength);

            // Bloom Settings
            ar.Property("bloomThreshold", settings.bloomThreshold);
            ar.Property("bloomKnee", settings.bloomKnee);
            ar.Property("bloomFilterRadius", settings.bloomFilterRadius);
            ar.Property("bloomExposure", settings.bloomExposure);
            ar.Property("bloomStrength", settings.bloomStrength);

            // Debug Visibility
            ar.Property("enableDebugVisibility", settings.enableDebugVisibility);
            ar.Property("debugVisibilityMode", reinterpret_cast<std::underlying_type_t<DebugVisibilityMode>&>(settings.debugVisibilityMode));

            // SSAO / HBAO Parameters
            ar.Property("aoRadius", settings.aoRadius);
            ar.Property("aoIntensity", settings.aoIntensity);
            ar.Property("maxOcclusionDistance", settings.maxOcclusionDistance);
            ar.Property("depthSharpness", settings.depthSharpness);
            ar.Property("bias", settings.bias);
            ar.Property("sampleCount", settings.sampleCount);

            ar.Property("enableSsao", settings.enableSsao);
            ar.Property("enableSsaoLight", settings.enableSsaoLight);
        }
    };
}