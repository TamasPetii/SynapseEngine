#ifndef COMMON_RENDER_DEFAULT_COLLIDER_GLSL
#define COMMON_RENDER_DEFAULT_COLLIDER_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

const uint COLLIDER_MODEL_BIT = 1u << 0u;

struct DefaultCollider
{
	vec3 aabbOrigin;
	uint objectIndex;
	vec3 aabbExtents;
	uint bitflag;
	vec3 sphereOrigin;
	float sphereRadius;
};


layout(buffer_reference, std430) readonly buffer DefaultColliderBuffer {
	DefaultCollider colliders[];
};

#endif