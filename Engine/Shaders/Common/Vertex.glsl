#ifndef COMMON_VERTEX_GLSL
#define COMMON_VERTEX_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct Vertex {
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec3 tangent;
	uint matIndex;
	vec3 bitangent;
	uint nodeIndex;
}; 

layout(buffer_reference, std430) readonly buffer VertexBuffer { 
	Vertex vertices[];
};

#endif