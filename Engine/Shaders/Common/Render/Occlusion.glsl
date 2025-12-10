#ifndef COMMON_RENDER_OCCLUSION_GLSL
#define COMMON_RENDER_OCCLUSION_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

layout(buffer_reference, std430) readonly buffer OcclusionBuffer {
	uint indices[];
};

#endif