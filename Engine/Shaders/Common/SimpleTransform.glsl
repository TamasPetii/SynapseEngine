#ifndef COMMON_SIMPLE_TRANSFORM_GLSL
#define COMMON_SIMPLE_TRANSFORM_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

layout(buffer_reference, std430) readonly buffer SimpleTransformBuffer { 
	mat4 transforms[];
};

#endif