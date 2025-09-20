#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require
#extension GL_EXT_nonuniform_qualifier : require

struct SpotLight {
	vec3 color;
	float strength;
	vec3 position;
	float shininess;
	vec3 direction;
	float len;
	vec4 angles;
	vec3 padding;
	uint bitflag;
}; 

layout(buffer_reference, std430) readonly buffer SpotLightBuffer { 
	SpotLight lights[];
};

//layout(set = 1, binding = 1) uniform texture2D u_spotLightShadowTextures[];