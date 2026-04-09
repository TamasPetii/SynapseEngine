#ifndef SYN_INCLUDES_COMMON_SPOT_LIGHT_GLSL
#define SYN_INCLUDES_COMMON_SPOT_LIGHT_GLSL

#include "../Core.glsl"

struct SpotLightComponent {
    mat4 transform;
    vec3 position;
    float range;
    vec3 direction;
    float weakenDistance;
    vec3 color;
    float strength;
    float innerAngle;
    float outerAngle;
    float innerCosAngle;
    float outerCosAngle;
    uint flags;
    uint padding0;
    uint padding1;
    uint padding2;
};

struct SpotLightColliderGPU {
    vec3 center;
    float radius;
    vec3 aabbMin;
    uint entityIndex;
    vec3 aabbMax;
    uint padding;
};

struct SpotLightShadowComponent {
    vec4 planes;
    mat4 viewProj;
    vec4 atlasRect;
};

layout(buffer_reference, std430) readonly restrict buffer SpotLightDataBuffer { SpotLightComponent data[]; };
layout(buffer_reference, std430) readonly restrict buffer SpotLightColliderDataBuffer { SpotLightColliderGPU data[]; };
layout(buffer_reference, std430) readonly restrict buffer SpotLightShadowDataBuffer { SpotLightShadowComponent data[]; };
layout(buffer_reference, std430) readonly restrict buffer VisibleSpotLightBuffer { uint data[]; };

#define GET_SPOT_LIGHT(addr, idx)        SpotLightDataBuffer(addr).data[idx]
#define GET_SPOT_LIGHT_COLLIDER(addr, idx) SpotLightColliderDataBuffer(addr).data[idx]
#define GET_SPOT_LIGHT_SHADOW(addr, idx) SpotLightShadowDataBuffer(addr).data[idx]
#define GET_VISIBLE_SPOT_LIGHT(addr, idx) VisibleSpotLightBuffer(addr).data[idx]

#endif