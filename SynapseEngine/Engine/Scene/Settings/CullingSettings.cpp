#include "CullingSettings.h"

namespace Syn
{
    CullingSettings::CullingSettings()
        : geometryCullingDevice(CullingDeviceType::CPU)
        , spotLightCullingDevice(CullingDeviceType::CPU)
        , pointLightCullingDevice(CullingDeviceType::CPU)
        , directionLightShadowCullingDevice(CullingDeviceType::CPU)
        , spotLightShadowCullingDevice(CullingDeviceType::CPU)
        , pointLightShadowCullingDevice(CullingDeviceType::CPU)
        , geometrySpatialAcceleration(SpatialAccelerationType::None)
        , directionLightShadowSpatialAcceleration(SpatialAccelerationType::None)
        , spotLightShadowSpatialAcceleration(SpatialAccelerationType::None)
        , pointLightShadowSpatialAcceleration(SpatialAccelerationType::None)
        , enableHiz(true)
        , enableMeshletConeCulling(true)
        , enableFrustumCulling(true)
        , enableChunkFrustumCulling(true)
        , enableModelFrustumCulling(true)
        , enableMeshFrustumCulling(true)
        , enableMeshletFrustumCulling(true)
        , enablePointLightFrustumCulling(true)
        , enableSpotLightFrustumCulling(true)
        , enableOcclusionCulling(true)
        , enableChunkOcclusionCulling(true)
        , enableModelOcclusionCulling(true)
        , enableMeshOcclusionCulling(true)
        , enableMeshletOcclusionCulling(true)
        , enablePointLightOcclusionCulling(true)
        , enableSpotLightOcclusionCulling(true)
    {}
}