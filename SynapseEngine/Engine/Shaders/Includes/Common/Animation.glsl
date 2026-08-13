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

#ifndef SYN_INCLUDES_COMMON_ANIMATION_GLSL
#define SYN_INCLUDES_COMMON_ANIMATION_GLSL

#include "../Core.glsl"
#include "Mesh.glsl"

struct GpuVertexSkinData {
    uvec4 boneIndices;
    vec4 boneWeights;
};

struct AnimationComponent {
    uint animationIndex;
    uint frameIndex;
    uint padding0;
    uint padding1;
};

struct GpuAnimationDescriptor {
    uint frameCount;
    uint nodeCount;
    uint globalVertexCount;
    uint globalMeshCount;
    uint globalMeshletCount;
    float durationInSeconds;
    float sampleRate;
    float padding;
};

struct GpuAnimationAddresses {
    uint isReady;
    uint padding;
    uint64_t vertexSkinData;
    uint64_t nodeTransforms;
    uint64_t frameGlobalColliders;
    uint64_t frameMeshColliders;
    uint64_t frameMeshletColliders;
    GpuAnimationDescriptor descriptor;
    GpuMeshCollider globalCollider;
};

layout(buffer_reference, std430) readonly restrict buffer VertexSkinDataBuffer     { GpuVertexSkinData data[]; };
layout(buffer_reference, std430) readonly restrict buffer AnimationComponentBuffer { AnimationComponent data[]; };
layout(buffer_reference, std430) readonly restrict buffer AnimationAddressBuffer   { GpuAnimationAddresses data[]; };

#define GET_SKIN_DATA(addr, idx)        VertexSkinDataBuffer(addr).data[idx]
#define GET_ANIM_COMP(addr, idx)        AnimationComponentBuffer(addr).data[idx]
#define GET_ANIM_ADDRESSES(addr, idx)   AnimationAddressBuffer(addr).data[idx]

#endif