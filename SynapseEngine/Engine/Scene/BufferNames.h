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
    struct SYN_API BufferNames
    {
        static constexpr const char* MortonKeysData = "MortonKeysData";
        static constexpr const char* MortonValuesData = "MortonValuesData";
        static constexpr const char* MortonChunkData = "MortonChunkData";
        static constexpr const char* MortonChunkVisibileIndex = "MortonChunkVisibileIndex";
        static constexpr const char* MortonChunkTransformsIndex = "MortonChunkTransformsIndex";
        
        static constexpr const char* TransformSparseMap = "TransformSparseMap";
        static constexpr const char* TransformData = "TransformData";
        static constexpr const char* TransformModelLinkData = "TransformModelLinkData";

        static constexpr const char* CameraSparseMap = "CameraSparseMap";
        static constexpr const char* CameraData = "CameraData";
        static constexpr const char* CameraVisibleData = "CameraVisibleData";

        static constexpr const char* ModelSparseMap = "ModelSparseMap";
        static constexpr const char* ModelData = "ModelData";
        static constexpr const char* ModelVisibleData = "ModelVisibleData";

        static constexpr const char* AnimationSparseMap = "AnimationSparseMap";
        static constexpr const char* AnimationData = "AnimationData";

        static constexpr const char* DirectionLightSparseMap = "DirectionLightSparseMap";
        static constexpr const char* DirectionLightData = "DirectionLightData";
        static constexpr const char* DirectionLightVisibleData = "DirectionLightVisibleData";
        static constexpr const char* DirectionLightVisibleShadowData = "DirectionLightVisibleShadowData";

        static constexpr const char* DirectionLightShadowSparseMap = "DirectionLightShadowSparseMap";
        static constexpr const char* DirectionLightShadowData = "DirectionLightShadowData";
        static constexpr const char* DirectionLightShadowColliderData = "DirectionLightShadowColliderData";
        static constexpr const char* DirectionLightShadowModelVisibleData = "DirectionLightShadowModelVisibleData";
        static constexpr const char* DirectionLightShadowMortonChunkVisibleIndex = "DirectionLightShadowMortonChunkVisibleIndex";
        static constexpr const char* DirectionLightShadowStaticChunkVisibleIndex = "DirectionLightShadowStaticChunkVisibleIndex";

        static constexpr const char* PointLightSparseMap = "PointLightSparseMap";
        static constexpr const char* PointLightData = "PointLightData";
        static constexpr const char* PointLightColliderData = "PointLightColliderData";
		static constexpr const char* PointLightVisibleData = "PointLightVisibleData";
        
        static constexpr const char* PointLightShadowSparseMap = "PointLightShadowSparseMap";
        static constexpr const char* PointLightShadowData = "PointLightShadowData";
        static constexpr const char* PointLightShadowVisibleData = "PointLightShadowVisibleData";
        static constexpr const char* PointLightShadowModelVisibleData = "PointLightShadowModelVisibleData";
        static constexpr const char* PointLightShadowMortonChunkVisibleIndex = "PointLightShadowMortonChunkVisibleIndex";
        static constexpr const char* PointLightShadowStaticChunkVisibleIndex = "PointLightShadowStaticChunkVisibleIndex";
        static constexpr const char* PointLightShadowAtlasSortKeyBuffer = "PointLightShadowAtlasSortKeyBuffer";
        static constexpr const char* PointLightShadowAtlasSortValueBuffer = "PointLightShadowAtlasSortValueBuffer";

        static constexpr const char* SpotLightSparseMap = "SpotLightSparseMap";
        static constexpr const char* SpotLightData = "SpotLightData";
        static constexpr const char* SpotLightColliderData = "SpotLightColliderData";
        static constexpr const char* SpotLightVisibleData = "SpotLightVisibleData";

        static constexpr const char* SpotLightShadowSparseMap = "SpotLightShadowSparseMap";
        static constexpr const char* SpotLightShadowData = "SpotLightShadowData";
        static constexpr const char* SpotLightShadowVisibleData = "SpotLightShadowVisibleData";
        static constexpr const char* SpotLightShadowModelVisibleData = "SpotLightShadowModelVisibleData";
        static constexpr const char* SpotLightShadowMortonChunkVisibleIndex = "SpotLightShadowMortonChunkVisibleIndex";
        static constexpr const char* SpotLightShadowStaticChunkVisibleIndex = "SpotLightShadowStaticChunkVisibleIndex";
        static constexpr const char* SpotLightShadowAtlasSortKeyBuffer = "SpotLightShadowAtlasSortKeyBuffer";
        static constexpr const char* SpotLightShadowAtlasSortValueBuffer = "SpotLightShadowAtlasSortValueBuffer";

        static constexpr const char* BoxColliderSparseMap = "BoxColliderSparseMap";
        static constexpr const char* BoxColliderData = "BoxColliderData";

        static constexpr const char* SphereColliderSparseMap = "SphereColliderSparseMap";
        static constexpr const char* SphereColliderData = "SphereColliderData";

        static constexpr const char* CapsuleColliderSparseMap = "CapsuleColliderSparseMap";
        static constexpr const char* CapsuleColliderData = "CapsuleColliderData";

        static constexpr const char* ConvexColliderSparseMap = "ConvexColliderSparseMap";
        static constexpr const char* ConvexColliderData = "ConvexColliderData";

        static constexpr const char* MeshColliderSparseMap = "MeshColliderSparseMap";
        static constexpr const char* MeshColliderData = "MeshColliderData";

        static constexpr const char* SelectionOutlineData = "SelectionOutlineData";
        static constexpr const char* HierarchySparseMap = "HierarchySparseMap";

        static constexpr const char* TagSparseMap = "TagSparseMap";
        static constexpr const char* TagData = "TagData";
    };
}