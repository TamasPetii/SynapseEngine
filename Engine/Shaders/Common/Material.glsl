#ifndef COMMON_MATERIAL_GLSL
#define COMMON_MATERIAL_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

#define INVALID_IMAGE_INDEX 0xFFFFFFFFu

struct Material {
	vec4 color;
	vec2 uvScale;
	vec2 bloom;
	uint albedoIndex;
	uint normalIndex;
	uint metallicIndex;
	uint roughnessIndex;
	float metalness;
	float roughness;
	vec2 padding;
}; 

layout(buffer_reference, std430) readonly buffer MaterialBuffer { 
	Material materials[];
};

layout(buffer_reference, std430) readonly buffer MaterialIndexBuffer { 
	uint materialIndices[];
};

#endif