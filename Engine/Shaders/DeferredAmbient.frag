#version 460

layout(location = 0) in vec2 fs_in_tex;
layout(location = 0) out vec4 fs_out_col;

layout(set = 0, binding = 0) uniform sampler2D u_colorTexture;
layout(set = 0, binding = 1) uniform sampler2D u_ambientOcclusionTexture;

void main()
{
	vec3 albedo = texture(u_colorTexture, fs_in_tex).xyz;
	float ao = texture(u_ambientOcclusionTexture, fs_in_tex).x;
	fs_out_col = vec4(vec3(0.05) * albedo * ao, 1); //[Todo] : Make the multiplyer push constant!
}