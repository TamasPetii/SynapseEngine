#ifndef COMMON_COUNT_GLSL
#define COMMON_COUNT_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

layout(buffer_reference, std430) buffer CountBuffer { 
	uint count;
};

#endif