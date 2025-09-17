#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require
#extension GL_EXT_nonuniform_qualifier : require

const uint SHADOW_BIT   = 0u;
const uint FALLOFF_BIT  = 1u;

struct PointLight {
	vec3 color;
	float strength;
	vec3 position;
	float shininess;
	float radius;
	float weakenDistance;
	uint bitflag;
	uint padding;
}; 

layout(buffer_reference, std430) readonly buffer PointLightBuffer { 
	PointLight lights[];
};

//layout(set = 1, binding = 1) uniform texture2D u_pointLightShadowTextures[];