// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "CullingSettings.h"

namespace Syn
{
    CullingSettings::CullingSettings()
        : geometryCullingDevice(CullingDeviceType::GPU)
        , spotLightCullingDevice(CullingDeviceType::CPU)
        , pointLightCullingDevice(CullingDeviceType::CPU)
        , directionLightShadowCullingDevice(CullingDeviceType::GPU)
        , spotLightShadowCullingDevice(CullingDeviceType::GPU)
        , pointLightShadowCullingDevice(CullingDeviceType::GPU)
        , geometrySpatialAcceleration(SpatialAccelerationType::MortonBvh)
        , directionLightShadowSpatialAcceleration(SpatialAccelerationType::MortonBvh)
        , spotLightShadowSpatialAcceleration(SpatialAccelerationType::MortonBvh)
        , pointLightShadowSpatialAcceleration(SpatialAccelerationType::MortonBvh)
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