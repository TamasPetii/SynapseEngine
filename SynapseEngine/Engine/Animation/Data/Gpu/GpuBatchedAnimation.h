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
#include "Engine/Animation/Data/Common/VertexSkinData.h"
#include "Engine/Mesh/Data/Gpu/GpuIndexedDrawData.h"
#include "Engine/Mesh/Data/Gpu/GpuMeshletDrawData.h"
#include "Engine/Mesh/Data/Gpu/GpuNodeTransform.h"
#include "Engine/Animation/Data/Common/BoneTrack.h"
#include "Engine/Animation/Data/Common/AnimationNode.h"
#include <vector>
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API GpuAnimationDescriptor
    {
        uint32_t frameCount;
        uint32_t nodeCount;
        uint32_t globalVertexCount;
        uint32_t globalMeshCount;
        uint32_t globalMeshletCount;

        float durationInSeconds;
        float sampleRate;
        float padding;
    };

    struct SYN_API GpuBatchedAnimation
    {
        GpuAnimationDescriptor descriptor;

        GpuMeshCollider globalCollider;

        // Flattened by meshNodeDescriptors!
        std::vector<VertexSkinData> vertexSkinData;

        // [Frame0_Node0... Frame0_NodeN | Frame1_Node0... Frame1_NodeN]
        std::vector<GpuNodeTransform> nodeTransforms;

        // [Frame0, Frame1, Frame2...]
        std::vector<GpuMeshCollider> frameGlobalColliders;

        // [Frame0_Instance0... Frame0_InstanceN | Frame1_Instance0... ]
        std::vector<GpuMeshCollider> frameMeshColliders;

        // [Frame0_Meshlet0... Frame0_MeshletN | Frame1_Meshlet0... ]
        std::vector<GpuMeshletCollider> frameMeshletColliders;

        std::vector<BoneTrack> tracks;
        std::vector<AnimationNode> nodes;
    };
}