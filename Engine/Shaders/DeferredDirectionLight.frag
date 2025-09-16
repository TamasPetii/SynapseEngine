#version 460

#include "Common/Camera.glsl"
#include "Common/Light/DirectionLight.glsl"
#include "Common/Pbr.glsl"

layout(location = 0) in vec2 fs_in_tex;
layout(location = 1) in flat uint fs_in_id;

layout(location = 0) out vec4 fs_out_col;

layout(set = 0, binding = 1) uniform sampler2D u_positionTexture;
layout(set = 0, binding = 2) uniform sampler2D u_colorTexture;
layout(set = 0, binding = 3) uniform sampler2D u_normalTexture;

layout( push_constant ) uniform constants
{	
	uvec2 cameraBuffer;
	uvec2 directionLightBuffer;
	vec3 padding;
	uint cameraIndex;
} PushConstants;

float EvalDirlightAttenuation()
{
	return 1.0;
}

void main()
{
	vec3 position = texture(u_positionTexture, fs_in_tex).xyz;

	vec4 albedoMetal = texture(u_colorTexture, fs_in_tex);
	vec4 normalRough = texture(u_normalTexture, fs_in_tex);

	vec3 albedo = albedoMetal.xyz;
	vec3 normal = normalize(normalRough.xyz);
	float metalness = albedoMetal.w;
	float roughness = normalRough.w;

	DirectionLightBuffer dlBuffer = DirectionLightBuffer(PushConstants.directionLightBuffer);

	vec3 toEye = normalize(CameraBuffer(PushConstants.cameraBuffer).cameras[PushConstants.cameraIndex].eye.xyz);
	vec3 toLight = normalize(-dlBuffer.lights[fs_in_id].direction);
	vec3 lightColor = dlBuffer.lights[fs_in_id].color;
	float attenuation = EvalDirlightAttenuation();
	
	vec3 Lo = ShadePhysicallyBased(albedo, normal, toEye, toLight, roughness, metalness, lightColor, attenuation);

	//Todo: Post process gamma correction + ambient in other shader

	fs_out_col = vec4(Lo, 1);
}