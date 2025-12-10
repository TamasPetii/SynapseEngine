#ifndef COMMON_ANIMATION_VERTEX_BONE_GLSL
#define COMMON_ANIMATION_VERTEX_BONE_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require
#define INVALID_VERTEX_BONE_INDEX 0xFFFFFFFFu

struct AnimationVertexBone {
	uvec4 indices;
	vec4 weights;
}; 

layout(buffer_reference, std430) readonly buffer AnimationVertexBoneBuffer { 
	AnimationVertexBone vertexBones[];
};

#endif