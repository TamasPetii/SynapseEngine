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