#pragma once
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Scene/Settings/CullingSettings.h"
#include "Engine/Scene/Settings/LightingSettings.h"
#include "Engine/Scene/Settings/PostProcessSettings.h"
#include "Engine/Scene/Settings/DebugSettings.h"
#include "Engine/Scene/Settings/SceneSettings.h"
#include <type_traits>

namespace Syn
{
    template <>
    struct Schema<CullingSettings>
    {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& settings = const_cast<std::remove_const_t<U>&>(val);

            // Hardware Devices
            ar.Property("geometryCullingDevice", settings.geometryCullingDevice);
            ar.Property("spotLightCullingDevice", settings.spotLightCullingDevice);
            ar.Property("pointLightCullingDevice", settings.pointLightCullingDevice);
            ar.Property("directionLightShadowCullingDevice", settings.directionLightShadowCullingDevice);
            ar.Property("spotLightShadowCullingDevice", settings.spotLightShadowCullingDevice);
            ar.Property("pointLightShadowCullingDevice", settings.pointLightShadowCullingDevice);

            // Acceleration & Framework Flags
            ar.Property("geometrySpatialAcceleration", settings.geometrySpatialAcceleration);
            ar.Property("directionLightShadowSpatialAcceleration", settings.directionLightShadowSpatialAcceleration);
            ar.Property("spotLightShadowSpatialAcceleration", settings.spotLightShadowSpatialAcceleration);
            ar.Property("pointLightShadowSpatialAcceleration", settings.pointLightShadowSpatialAcceleration);
            
            ar.Property("enableHiz", settings.enableHiz);
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
        }
    };

    template <>
    struct Schema<LightingSettings>
    {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& settings = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("pipelineType", settings.pipelineType);
            ar.Property("tileSize", settings.tileSize);
            ar.Property("ambientStrength", settings.ambientStrength);
            ar.Property("emissiveStrength", settings.emissiveStrength);

            // Deferred Features Toggles
            ar.Property("enableDeferredEmissiveAo", settings.enableDeferredEmissiveAo);
            ar.Property("enableDeferredPointLights", settings.enableDeferredPointLights);
            ar.Property("enableDeferredSpotLights", settings.enableDeferredSpotLights);
            ar.Property("enableDeferredDirectionalLights", settings.enableDeferredDirectionalLights);

            // Forward+ Features Toggles
            ar.Property("enableForwardPlusEmissiveAo", settings.enableForwardPlusEmissiveAo);
            ar.Property("enableForwardPlusPointLights", settings.enableForwardPlusPointLights);
            ar.Property("enableForwardPlusSpotLights", settings.enableForwardPlusSpotLights);
            ar.Property("enableForwardPlusDirectionalLights", settings.enableForwardPlusDirectionalLights);
        }
    };

    template <>
    struct Schema<PostProcessSettings>
    {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& settings = const_cast<std::remove_const_t<U>&>(val);

            // Bloom
            ar.Property("enableBloom", settings.enableBloom);
            ar.Property("bloomThreshold", settings.bloomThreshold);
            ar.Property("bloomKnee", settings.bloomKnee);
            ar.Property("bloomFilterRadius", settings.bloomFilterRadius);
            ar.Property("bloomExposure", settings.bloomExposure);
            ar.Property("bloomStrength", settings.bloomStrength);

            // SSAO
            ar.Property("enableSsao", settings.enableSsao);
            ar.Property("enableSsaoLight", settings.enableSsaoLight);
            ar.Property("aoRadius", settings.aoRadius);
            ar.Property("aoIntensity", settings.aoIntensity);
            ar.Property("maxOcclusionDistance", settings.maxOcclusionDistance);
            ar.Property("depthSharpness", settings.depthSharpness);
            ar.Property("bias", settings.bias);
            ar.Property("sampleCount", settings.sampleCount);
        }
    };

    template <>
    struct Schema<DebugSettings>
    {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& settings = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("useDebugCamera", settings.useDebugCamera);
            ar.Property("enableDebugVisibility", settings.enableDebugVisibility);
            ar.Property("debugVisibilityMode", settings.debugVisibilityMode);

            // Geometry Wireframes
            ar.Property("enableWireframeMeshAabb", settings.enableWireframeMeshAabb);
            ar.Property("enableWireframeMeshSphere", settings.enableWireframeMeshSphere);
            ar.Property("enableWireframeMeshletAabb", settings.enableWireframeMeshletAabb);
            ar.Property("enableWireframeMeshletSphere", settings.enableWireframeMeshletSphere);
            ar.Property("enableWireframeMeshletCone", settings.enableWireframeMeshletCone);
            ar.Property("enableStaticChunkAabbWireframe", settings.enableStaticChunkAabbWireframe);
            ar.Property("enableMortonChunkAabbWireframe", settings.enableMortonChunkAabbWireframe);

            // Light Wireframes
            ar.Property("enablePointLightSphereWireframe", settings.enablePointLightSphereWireframe);
            ar.Property("enablePointLightAabbWireframe", settings.enablePointLightAabbWireframe);
            ar.Property("enableSpotLightSphereWireframe", settings.enableSpotLightSphereWireframe);
            ar.Property("enableSpotLightAabbWireframe", settings.enableSpotLightAabbWireframe);
            ar.Property("enableSpotLightConeWireframe", settings.enableSpotLightConeWireframe);
            ar.Property("enableSpotLightPyramidWireframe", settings.enableSpotLightPyramidWireframe);

            // Physics Colliders
            ar.Property("enablePhysicsWireframe", settings.enablePhysicsWireframe);

            // Billboards
            ar.Property("enableBillboardCameras", settings.enableBillboardCameras);
            ar.Property("enableBillboardPointLights", settings.enableBillboardPointLights);
            ar.Property("enableBillboardSpotLights", settings.enableBillboardSpotLights);
            ar.Property("enableBillboardDirectionalLights", settings.enableBillboardDirectionalLights);

            // Selection Outlines
            ar.Property("enableSelectedOutline", settings.enableSelectedOutline);
            ar.Property("enableSelectedHierarchyOutline", settings.enableSelectedHierarchyOutline);
            ar.Property("outlinePrimaryColor", settings.outlinePrimaryColor);
            ar.Property("outlineSecondaryColor", settings.outlineSecondaryColor);
            ar.Property("outlineThickness", settings.outlineThickness);

            // Infinite Grid
            ar.Property("enableInfiniteGrid", settings.enableInfiniteGrid);
            ar.Property("gridScale", settings.gridScale);
            ar.Property("fadeDistance", settings.fadeDistance);
            ar.Property("gridThickness", settings.gridThickness);
            ar.Property("axisThickness", settings.axisThickness);
            ar.Property("gridShowXZ", settings.gridShowXZ);
            ar.Property("gridShowXY", settings.gridShowXY);
            ar.Property("gridShowYZ", settings.gridShowYZ);
            ar.Property("gridShowAxisX", settings.gridShowAxisX);
            ar.Property("gridShowAxisY", settings.gridShowAxisY);
            ar.Property("gridShowAxisZ", settings.gridShowAxisZ);
            ar.Property("gridColor", settings.gridColor);
            ar.Property("axisXColor", settings.axisXColor);
            ar.Property("axisYColor", settings.axisYColor);
            ar.Property("axisZColor", settings.axisZColor);
        }
    };

    template <>
    struct Schema<SceneSettings>
    {
        static constexpr bool exists = true;

        template <typename Archive, typename U>
        static void Invoke(Archive& ar, const char* name, U& val)
        {
            ScopedArchiveObject obj(ar, name);
            auto& settings = const_cast<std::remove_const_t<U>&>(val);

            ar.Property("Culling", settings.culling);
            ar.Property("Lighting", settings.lighting);
            ar.Property("PostProcess", settings.postProcess);
            ar.Property("Debug", settings.debug);
        }
    };
}