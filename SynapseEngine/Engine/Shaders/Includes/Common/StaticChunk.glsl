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

#ifndef SYN_INCLUDES_COMMON_STATIC_CHUNK_GLSL
#define SYN_INCLUDES_COMMON_STATIC_CHUNK_GLSL

#include "../Core.glsl"

#define CHUNK_SIZE 32

const float AABB_INFINITY = 1e30;
const vec3 BOUNDS_INIT_MIN = vec3(AABB_INFINITY);
const vec3 BOUNDS_INIT_MAX = vec3(-AABB_INFINITY);

struct StaticChunk {
    vec3 minBounds;
    uint firstEntityIndex;
    vec3 maxBounds;
    uint entityCount;
};

struct SceneAABB {
    uint minX; uint minY; uint minZ;
    uint maxX; uint maxY; uint maxZ;
};

layout(buffer_reference, std430) readonly restrict buffer StaticChunkBuffer { 
    StaticChunk data[];
};

layout(buffer_reference, std430) restrict buffer VisibleChunkList { 
    uint data[];
};

layout(buffer_reference, std430) restrict buffer VisibleChunkListUvec2 { 
    uvec2 data[];
};

layout(buffer_reference, std430) restrict buffer SceneAABBBuffer { 
    SceneAABB data; 
};

layout(buffer_reference, std430) restrict buffer MortonKeysBuffer { 
    uint data[]; 
};

layout(buffer_reference, std430) restrict buffer MortonValueBuffer { 
    uint data[]; 
};

layout(buffer_reference, std430) restrict buffer ChunkTransformIndicesBuffer { 
    uint data[]; 
};



#define GET_STATIC_CHUNK(addr, idx)         StaticChunkBuffer(addr).data[idx]
#define GET_VISIBLE_CHUNK(addr, idx)        VisibleChunkList(addr).data[idx]
#define GET_VISIBLE_CHUNK_UVEC2(addr, idx)  VisibleChunkListUvec2(addr).data[idx]

#define GET_SCENE_AABB(addr)            SceneAABBBuffer(addr).data
#define GET_MORTON_KEY(addr, idx)       MortonKeysBuffer(addr).data[idx]
#define GET_MORTON_VALUE(addr, idx)     MortonValueBuffer(addr).data[idx]
#define GET_CHUNK_TRANSFORM_IDX(addr, idx)     ChunkTransformIndicesBuffer(addr).data[idx]

#endif