#ifndef COMMON_LIGHT_COMMON_GLSL
#define COMMON_LIGHT_COMMON_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct LightCommonData {
	uvec3 aabbMin;
	uint count;
	uvec3 aabbMax;
	uint shadowCount;
	vec3 padding;
	uint objectCount;
};

layout(buffer_reference, std430) buffer LightCommonDataBuffer { 
	LightCommonData commonData;
};

#endif