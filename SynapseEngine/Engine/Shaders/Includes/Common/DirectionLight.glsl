#ifndef SYN_INCLUDES_COMMON_DIRECTION_LIGHT_GLSL
#define SYN_INCLUDES_COMMON_DIRECTION_LIGHT_GLSL

#include "../Core.glsl"

struct DirectionLightComponent {
    vec3 direction;
    float strength;
    vec3 color;
    uint flags;
};

struct DirectionLightShadowComponent {
    vec4 cascadeSplits;
    mat4 cascadeViewProjsVulkan[4];
    vec4 cascadeAtlasRects[4];
};

struct CascadeCollider {
    vec4 planes[6]; // Near, Far, Left, Right, Top, Bottom
    vec4 aabbMin;
    vec4 aabbMax;
};

struct DirectionLightShadowColliderGPU {
    CascadeCollider cascades[4];
    uint entityIndex;
    uint padding[3];
};

layout(buffer_reference, std430) readonly restrict buffer DirectionLightDataBuffer { DirectionLightComponent data[]; };
layout(buffer_reference, std430) readonly restrict buffer DirectionLightShadowDataBuffer { DirectionLightShadowComponent data[]; };
layout(buffer_reference, std430) readonly restrict buffer DirectionLightShadowColliderDataBuffer { DirectionLightShadowColliderGPU data[]; };
layout(buffer_reference, std430) readonly restrict buffer DirectionVisibleLightBuffer { uint data[]; };
layout(buffer_reference, std430) readonly restrict buffer DirectionVisibleShadowLightBuffer { uint data[]; };

#define GET_DIRECTION_LIGHT(addr, idx)                 DirectionLightDataBuffer(addr).data[idx]
#define GET_DIRECTION_LIGHT_SHADOW(addr, idx)          DirectionLightShadowDataBuffer(addr).data[idx]
#define GET_DIRECTION_LIGHT_SHADOW_COLLIDER(addr, idx) DirectionLightShadowColliderDataBuffer(addr).data[idx]
#define GET_DIRECTION_VISIBLE_LIGHT(addr, idx)         DirectionVisibleLightBuffer(addr).data[idx]
#define GET_DIRECTION_VISIBLE_SHADOW_LIGHT(addr, idx)  DirectionVisibleShadowLightBuffer(addr).data[idx]

#endif