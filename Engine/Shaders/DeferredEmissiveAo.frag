#version 460

layout(location = 0) in vec2 fs_in_tex;
layout(location = 0) out vec4 fs_out_col;

layout(set = 0, binding = 2) uniform sampler2D u_colorTexture;
layout(set = 0, binding = 5) uniform sampler2D u_emissiveAoTexture;

layout(push_constant) uniform constants {
    float emissiveStrength; // 0.0 disabled, 1.0 enabled
} PushConstants;

void main()
{
	vec3 albedo = texture(u_colorTexture, fs_in_tex).xyz;
	vec4 emissiveAo = texture(u_emissiveAoTexture, fs_in_tex);

	vec3 aoColor = vec3(0.05) * albedo * emissiveAo.w;
	vec3 emissiveColor = emissiveAo.xyz * PushConstants.emissiveStrength;

	fs_out_col = vec4(aoColor + emissiveColor, 1);
}