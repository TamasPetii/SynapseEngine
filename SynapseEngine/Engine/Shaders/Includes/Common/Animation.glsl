#ifndef SYN_INCLUDES_COMMON_ANIMATION_GLSL
#define SYN_INCLUDES_COMMON_ANIMATION_GLSL

#include "../Core.glsl"

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
    uint64_t vertexSkinData;
    uint64_t nodeTransforms;
    uint64_t frameGlobalColliders;
    uint64_t frameMeshColliders;
    uint64_t frameMeshletColliders;
    uint64_t padding;
    GpuAnimationDescriptor descriptor;
};

layout(buffer_reference, std430) readonly buffer VertexSkinDataBuffer     { GpuVertexSkinData data[]; };
layout(buffer_reference, std430) readonly buffer AnimationComponentBuffer { AnimationComponent data[]; };
layout(buffer_reference, std430) readonly buffer AnimationAddressBuffer   { GpuAnimationAddresses data[]; };

#define GET_SKIN_DATA(addr, idx)        VertexSkinDataBuffer(addr).data[idx]
#define GET_ANIM_COMP(addr, idx)        AnimationComponentBuffer(addr).data[idx]
#define GET_ANIM_ADDRESSES(addr, idx)   AnimationAddressBuffer(addr).data[idx]

#endif