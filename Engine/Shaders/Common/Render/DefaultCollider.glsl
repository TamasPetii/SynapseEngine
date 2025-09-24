#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct DefaultCollider
{
	vec3 aabbOrigin;
	uint32_t objectIndex;
	vec3 aabbExtents;
	uint32_t bitflag;
	vec3 sphereOrigin;
	float sphereRadius;
};


layout(buffer_reference, std430) readonly buffer DefaultColliderBuffer {
	DefaultCollider colliders[];
};

