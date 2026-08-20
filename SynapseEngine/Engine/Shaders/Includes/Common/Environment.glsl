#ifndef SYN_INCLUDES_COMMON_ENVIRONMENT_GLSL
#define SYN_INCLUDES_COMMON_ENVIRONMENT_GLSL

#include "../Core.glsl"
#include "Texture.glsl"

struct EnvironmentData {
    mat4 skyRotationMatrix;
    
    vec3 skyTint;
    float skyExposureEV;
    
    float intensity;
    float ambientIntensity;

    uint packedSamplers;
    uint skyTextureIndex;
};

layout(buffer_reference, std430, buffer_reference_align = 16) readonly restrict buffer EnvironmentBuffer {
    EnvironmentData data[];
};

#define GET_ENVIRONMENT(addr, index) EnvironmentBuffer(addr).data[index]
#define GET_ENV_CUBE_SAMPLER(env)   ((env).packedSamplers & 0xFFFF)
#define GET_ENV_SPHERE_SAMPLER(env) ((env).packedSamplers >> 16)

#define GET_ENV_BASE_CUBE_INDEX(envIndex)        ((envIndex) * 3 + 0)
#define GET_ENV_IRRADIANCE_CUBE_INDEX(envIndex)  ((envIndex) * 3 + 1)
#define GET_ENV_PREFILTERED_CUBE_INDEX(envIndex) ((envIndex) * 3 + 2)

#define SampleEnvironmentBaseCube(envIndex, samplerIndex, dir, lod) SampleTextureCubeLod(GET_ENV_BASE_CUBE_INDEX(envIndex), samplerIndex, dir, lod)
#define SampleEnvironmentIrradianceCube(envIndex, samplerIndex, dir) SampleTextureCube(GET_ENV_IRRADIANCE_CUBE_INDEX(envIndex), samplerIndex, dir)
#define SampleEnvironmentPrefilteredCube(envIndex, samplerIndex, dir, lod) SampleTextureCubeLod(GET_ENV_PREFILTERED_CUBE_INDEX(envIndex), samplerIndex, dir, lod)

#endif