#ifndef COMMON_MODEL_NODE_TRANSFORM_GLSL
#define COMMON_MODEL_NODE_TRANSFORM_GLSL

#extension GL_EXT_buffer_reference : require

struct ModelNodeTransform {
	mat4 transform;
	mat4 transformIT;
}; 

layout(buffer_reference, std430) readonly buffer ModelNodeTransformBuffer { 
	ModelNodeTransform nodeTransforms[];
};

#endif