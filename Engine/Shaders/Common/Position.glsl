#ifndef COMMON_POSITION_GLSL
#define COMMON_POSITION_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

layout(buffer_reference, std430) readonly buffer PositionBuffer { 
	vec4 positions[];
};

#endif