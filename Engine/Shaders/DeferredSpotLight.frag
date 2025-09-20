#version 460

#include "Common/Camera.glsl"
#include "Common/Light/SpotLight.glsl"
#include "Common/Pbr.glsl"

layout(location = 0) in flat uint fs_in_id;
layout(location = 0) out vec4 fs_out_col;

layout(set = 0, binding = 1) uniform sampler2D u_positionTexture;
layout(set = 0, binding = 2) uniform sampler2D u_colorTexture;
layout(set = 0, binding = 3) uniform sampler2D u_normalTexture;

layout( push_constant ) uniform constants
{	
	uvec2 cameraBuffer;
	uvec2 spotLightBuffer;
	uvec2 transformBufferAddress;
	uvec2 instanceBufferAddress;
	uvec2 vertexBufferAddress;
	uvec2 indexBufferAddress;
	vec2 viewPortSize;
	uint cameraIndex;
	uint padding;
} PushConstants;

float EvalSpotlightAttenuation(SpotLightBuffer slBuffer, uint lightIndex, vec3 toLight)
{
	float cosTheta = dot(normalize(slBuffer.lights[fs_in_id].direction), normalize(-toLight));

	if(cosTheta >= slBuffer.lights[fs_in_id].angles.z)
		return 1.0;

	float theta = degrees(acos(cosTheta));
	float angularAttenuation = clamp((slBuffer.lights[fs_in_id].angles.y - theta) / (slBuffer.lights[fs_in_id].angles.y - slBuffer.lights[fs_in_id].angles.x), 0.0, 1.0);

	float dist = length(toLight);
	float distanceAttenuation = 1.0 / (dist * dist);

	return angularAttenuation;
}

void main()
{
	SpotLightBuffer slBuffer = SpotLightBuffer(PushConstants.spotLightBuffer);

	vec2 fs_in_tex = gl_FragCoord.xy / PushConstants.viewPortSize;

	vec3 position = texture(u_positionTexture, fs_in_tex).xyz;

	vec3 fromSpotToFrag = position - slBuffer.lights[fs_in_id].position;
	vec3 fromSpotToFragNorm = normalize(fromSpotToFrag);
	vec3 spotDirNorm = normalize(slBuffer.lights[fs_in_id].direction);
	float alpha = dot(spotDirNorm, fromSpotToFragNorm); // Angle: tells if we are inside of the cone
	float delta = dot(spotDirNorm, fromSpotToFrag); //The projected length to check if its inside the far plane

	//Alpha <, we calculate with cosine!!!
	if(alpha < slBuffer.lights[fs_in_id].angles.w || delta > slBuffer.lights[fs_in_id].len)
		discard;

	vec4 albedoMetal = texture(u_colorTexture, fs_in_tex);
	vec4 normalRough = texture(u_normalTexture, fs_in_tex);

	vec3 albedo = albedoMetal.xyz;
	vec3 normal = normalize(normalRough.xyz);
	float metalness = albedoMetal.w;
	float roughness = normalRough.w;

	vec3 toEye = normalize(CameraBuffer(PushConstants.cameraBuffer).cameras[PushConstants.cameraIndex].eye.xyz - position);
	vec3 toLightNotNorm = slBuffer.lights[fs_in_id].position - position;
	vec3 toLight = normalize(toLightNotNorm);
	
	vec3 lightColor = slBuffer.lights[fs_in_id].color;
	float attenuation = EvalSpotlightAttenuation(slBuffer, fs_in_id, toLightNotNorm);
	
	vec3 Lo = ShadePhysicallyBased(albedo, normal, toEye, toLight, roughness, metalness, lightColor, attenuation, slBuffer.lights[fs_in_id].strength);

	//Todo: Post process gamma correction + ambient in other shader

	fs_out_col = vec4(Lo, 1);
}