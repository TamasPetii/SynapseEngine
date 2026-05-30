#ifndef SYN_INCLUDES_PC_DIRECTION_LIGHT_SHADOW_TRADITIONAL_MESHLET_PASS_GLSL
#define SYN_INCLUDES_PC_DIRECTION_LIGHT_SHADOW_TRADITIONAL_MESHLET_PASS_GLSL

#include "../SharedGpuTypes.glsl"

struct DirectionLightShadowTraditionalMeshletPassPC {
    uint64_t frameGlobalContextBufferAddr;
    uint baseDescriptorOffset;
    uint materialRenderType;
    uint disableConeCulling;
    uint shadowMultiplier;
};

#endif