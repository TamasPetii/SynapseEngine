#ifndef COMMON_LIGHT_SPOT_LIGHT_GLSL
#define COMMON_LIGHT_SPOT_LIGHT_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require
#extension GL_EXT_nonuniform_qualifier : require

const uint SHADOW_BIT   = 0u;
const uint FALLOFF_BIT  = 1u;

struct SpotLight {
	vec3 color;
	float strength;
	vec3 position;
	float shininess;
	vec3 direction;
	float range;
	vec4 angles;
	vec3 boundingSphereOrigin;
	float boundingSphereRadius;
	vec3 aabbOrigin;
	uint bitflag;
	vec3 aabbExtents;
	uint shadowIndex;
}; 

struct SpotLightShadow {
	float nearPlane;
	float farPlane;
	vec2 texSize;
	mat4 viewProj;
}; 

layout(buffer_reference, std430) readonly buffer SpotLightBuffer { 
	SpotLight lights[];
};

layout(buffer_reference, std430) readonly buffer SpotLightShadowBuffer { 
	SpotLightShadow lightShadows[];
};

//layout(set = 1, binding = 1) uniform texture2D u_spotLightShadowTextures[];

#endif