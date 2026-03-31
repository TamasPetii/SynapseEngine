#ifndef SYN_INCLUDES_COMMON_POINT_LIGHT_GLSL
#define SYN_INCLUDES_COMMON_POINT_LIGHT_GLSL

#include "../Core.glsl"

struct PointLightComponent {
    vec3 position;
    float radius;
    vec3 color;
    float strength;
    float weakenDistance;
    uint flags;
    uint padding;
    uint padding2;
};

struct PointLightColliderGPU
{
    vec3 center;
    float radius;
    uint entityIndex;
    uint padding0;
    uint padding1;
    uint padding2;
};

struct PointLightShadowComponent {
    vec4 planes;
    mat4 viewProjs[6];
    vec4 atlasRects[6];
};

layout(buffer_reference, std430) readonly buffer PointLightDataBuffer { PointLightComponent data[]; };
layout(buffer_reference, std430) readonly buffer PointLightColliderDataBuffer { PointLightColliderGPU data[]; };
layout(buffer_reference, std430) readonly buffer PointLightShadowDataBuffer { PointLightShadowComponent data[]; };
layout(buffer_reference, std430) readonly buffer VisiblePointLightBuffer   { uint data[]; };

#define GET_POINT_LIGHT(addr, idx)            PointLightDataBuffer(addr).data[idx]
#define GET_POINT_LIGHT_COLLIDER(addr, idx)   PointLightColliderDataBuffer(addr).data[idx]
#define GET_POINT_LIGHT_SHADOW(addr, idx)     PointLightShadowDataBuffer(addr).data[idx]
#define GET_VISIBLE_POINT_LIGHT(addr, idx)    VisiblePointLightBuffer(addr).data[idx]

#endif