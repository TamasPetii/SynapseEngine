#pragma once
#include "Engine/SynApi.h"

namespace Syn
{
    enum SYN_API CullingDeviceType
    {
        CPU,
        GPU
    };

    enum SYN_API SpatialAccelerationType
    {
        None,
        StaticBvh,
        MortonBvh
    };

    struct SYN_API CullingSettings
    {
        CullingSettings();

        CullingDeviceType geometryCullingDevice;
        CullingDeviceType spotLightCullingDevice;
        CullingDeviceType pointLightCullingDevice;
        CullingDeviceType directionLightShadowCullingDevice;
        CullingDeviceType spotLightShadowCullingDevice;
        CullingDeviceType pointLightShadowCullingDevice;

        SpatialAccelerationType geometrySpatialAcceleration;
        SpatialAccelerationType directionLightShadowSpatialAcceleration;
        SpatialAccelerationType spotLightShadowSpatialAcceleration;
        SpatialAccelerationType pointLightShadowSpatialAcceleration;

        bool enableHiz;
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
    };
}