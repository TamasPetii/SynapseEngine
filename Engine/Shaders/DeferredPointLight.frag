#version 460

#include "Common/Camera.glsl"
#include "Common/Light/PointLight.glsl"
#include "Common/Pbr.glsl"

layout(location = 0) in flat uint fs_in_id;
layout(location = 0) out vec4 fs_out_col;

layout(set = 0, binding = 1) uniform sampler2D u_positionTexture;
layout(set = 0, binding = 2) uniform sampler2D u_colorTexture;
layout(set = 0, binding = 3) uniform sampler2D u_normalTexture;

layout( push_constant ) uniform constants
{	
	uvec2 cameraBuffer;
	uvec2 pointLightBuffer;
	uvec2 transformBufferAddress;
	uvec2 instanceBufferAddress;
	uvec2 vertexBufferAddress;
	uvec2 indexBufferAddress;
	vec2 viewPortSize;
	uint cameraIndex;
	uint padding;
} PushConstants;

float EvalPointlightAttenuation(PointLightBuffer plBuffer, uint lightIndex, vec3 toLight)
{
	float attenuation = 1.0;
	float dist = length(toLight);

	float lightRadius = plBuffer.lights[lightIndex].radius;
	float lightWeakenDistance = plBuffer.lights[lightIndex].weakenDistance;
	
    if (dist <= lightWeakenDistance)
        return 1.0;

    if (dist >= lightRadius)
        return 0.0;

    float t = (dist - lightWeakenDistance) / (lightRadius - lightWeakenDistance);
    t = clamp(t, 0.0, 1.0);
	
	//Quadratic Interpolation -> Second bit in bitflag
    if (((plBuffer.lights[lightIndex].bitflag >> FALLOFF_BIT) & 1u) != 0) {
		return 1.0 - (t * t);	
    } 
	//Linear Interpolation
	else {
		return 1.0 - t;
    }
}

void main()
{
	PointLightBuffer plBuffer = PointLightBuffer(PushConstants.pointLightBuffer);

	vec2 fs_in_tex = gl_FragCoord.xy / PushConstants.viewPortSize;

	vec3 position = texture(u_positionTexture, fs_in_tex).xyz;

	//Box is rendered for pointlights, need to discard fragments that are outside of the sphere volume
	if(distance(position, plBuffer.lights[fs_in_id].position) > plBuffer.lights[fs_in_id].radius)
		discard;

	vec4 albedoMetal = texture(u_colorTexture, fs_in_tex);
	vec4 normalRough = texture(u_normalTexture, fs_in_tex);

	vec3 albedo = albedoMetal.xyz;
	vec3 normal = normalize(normalRough.xyz);
	float metalness = albedoMetal.w;
	float roughness = normalRough.w;

	vec3 toEye = normalize(CameraBuffer(PushConstants.cameraBuffer).cameras[PushConstants.cameraIndex].eye.xyz - position);
	vec3 toLightNotNorm = plBuffer.lights[fs_in_id].position - position;
	vec3 toLight = normalize(toLightNotNorm);
	
	vec3 lightColor = plBuffer.lights[fs_in_id].color;
	float attenuation = EvalPointlightAttenuation(plBuffer, fs_in_id, toLightNotNorm);
	
	vec3 Lo = ShadePhysicallyBased(albedo, normal, toEye, toLight, roughness, metalness, lightColor, attenuation, plBuffer.lights[fs_in_id].strength);

	//Todo: Post process gamma correction + ambient in other shader

	fs_out_col = vec4(Lo, 1);
}