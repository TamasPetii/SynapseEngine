#ifndef SYN_INCLUDES_COMMON_MATERIAL_GLSL
#define SYN_INCLUDES_COMMON_MATERIAL_GLSL

#include "../Core.glsl"

struct Material { 
    vec4 color; 
    vec3 emissiveColor; 
    float emissiveIntensity; 
    vec2 uvScale; 
    float metalness; 
    float roughness; 
    float aoStrength; 
    uint packedFlags; 
    uint albedoTexture; 
    uint normalTexture; 
    uint metalnessTexture; 
    uint roughnessTexture; 
    uint metallicRoughnessTexture; 
    uint emissiveTexture; 
    uint ambientOcclusionTexture; 
    uint padding0; 
    uint padding1; 
    uint padding2; 
};

layout(buffer_reference, std430) readonly buffer MaterialBuffer { Material data[]; };
layout(buffer_reference, std430) readonly buffer MaterialLookupBuffer { uint data[]; };

#define GET_MATERIAL(addr, idx)         MaterialBuffer(addr).data[idx]
#define GET_MATERIAL_INDEX(addr, idx)   MaterialLookupBuffer(addr).data[idx]

#define HAS_VALID_TEXTURE(texIdx) ((texIdx) != INVALID_INDEX)

#define IS_DOUBLE_SIDED(mat)    HAS_FLAG((mat).packedFlags, 0)
#define IS_TRANSPARENT(mat)     HAS_FLAG((mat).packedFlags, 1)

#define HAS_ALBEDO_TEX(mat)             HAS_VALID_TEXTURE((mat).albedoTexture)
#define HAS_NORMAL_TEX(mat)             HAS_VALID_TEXTURE((mat).normalTexture)
#define HAS_METALNESS_TEX(mat)          HAS_VALID_TEXTURE((mat).metalnessTexture)
#define HAS_ROUGHNESS_TEX(mat)          HAS_VALID_TEXTURE((mat).roughnessTexture)
#define HAS_METALLIC_ROUGHNESS_TEX(mat) HAS_VALID_TEXTURE((mat).metallicRoughnessTexture)
#define HAS_EMISSIVE_TEX(mat)           HAS_VALID_TEXTURE((mat).emissiveTexture)
#define HAS_AO_TEX(mat)                 HAS_VALID_TEXTURE((mat).ambientOcclusionTexture)

#endif

