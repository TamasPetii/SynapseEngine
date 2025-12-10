#ifndef COMMON_INDEX_GLSL
#define COMMON_INDEX_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

layout(buffer_reference, std430) readonly buffer IndexBuffer { 
	uint indices[];
};

#endif