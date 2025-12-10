#ifndef COMMON_TRANSFORM_GLSL
#define COMMON_TRANSFORM_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct Transform {
	mat4 transform;
	mat4 transformIT;
}; 

layout(buffer_reference, std430) readonly buffer TransformBuffer { 
	Transform transforms[];
};

#endif