#ifndef SYN_INCLUDES_COMMON_STATIC_CHUNK_GLSL
#define SYN_INCLUDES_COMMON_STATIC_CHUNK_GLSL

#include "../Core.glsl"

struct StaticChunk {
    vec3 minBounds;
    uint firstEntityIndex;
    vec3 maxBounds;
    uint entityCount;
};

layout(buffer_reference, std430) readonly restrict buffer StaticChunkBuffer { StaticChunk data[]; };
layout(buffer_reference, std430) restrict buffer VisibleChunkList { uint data[]; };

#define GET_STATIC_CHUNK(addr, idx)     StaticChunkBuffer(addr).data[idx]
#define GET_VISIBLE_CHUNK(addr, idx)    VisibleChunkList(addr).data[idx]

#endif