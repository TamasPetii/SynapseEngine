#ifndef SYN_INCLUDES_COMMON_COLLIDER_GLSL
#define SYN_INCLUDES_COMMON_COLLIDER_GLSL

#include "../Core.glsl"

struct BoxColliderComponent {
	vec3 halfExtents;
	uint entityIndex;
	vec3 localOffset;
	float pad0;
};

struct CapsuleColliderComponent {
    vec3 localOffset;
	float radius;
	float halfHeight;
	uint entityIndex;
	float _pad1;
	float _pad2;
};

struct SphereColliderComponent {
	vec3 localOffset;
	float radius;
	uint entityIndex;
	float pad0;
	float pad1;
	float pad2;
};

layout(buffer_reference, std430) readonly restrict buffer BoxColliderBuffer   { BoxColliderComponent data[]; };
layout(buffer_reference, std430) readonly restrict buffer CapsuleColliderBuffer { CapsuleColliderComponent data[]; };
layout(buffer_reference, std430) readonly restrict buffer SphereColliderBuffer { SphereColliderComponent data[]; };

#define GET_BOX_COLLIDER(addr, idx)      BoxColliderBuffer(addr).data[idx]
#define GET_CAPSULE_COLLIDER(addr, idx)  CapsuleColliderBuffer(addr).data[idx]
#define GET_SPHERE_COLLIDER(addr, idx)   SphereColliderBuffer(addr).data[idx]

#endif