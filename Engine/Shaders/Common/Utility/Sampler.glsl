#ifndef SAMPLER_GLSL
#define SAMPLER_GLSL

#extension GL_EXT_nonuniform_qualifier : require

#define NEAREST_SAMPLER_ID 0
#define LINEAR_SAMPLER_ID 1
#define NEAREST_ANISOTROPY_SAMPLER_ID 2
#define LINEAR_ANISOTROPY_SAMPLER_ID 3
#define REDUCTION_MAX_SAMPLER_ID 4

layout(set = 0, binding = 0) uniform sampler u_samplers[];

#endif