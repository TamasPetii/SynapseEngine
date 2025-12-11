#version 460
#extension GL_EXT_buffer_reference_uvec2 : require

#include "Common/Utility/Sampler.glsl"
#include "Common/Utility/TextureLoaded.glsl"
#include "Common/Material.glsl"
#include "Common/Render/RenderDefines.glsl"
#include "Common/Model/ModelBufferAddresses.glsl"

//Inputs
layout (location = 0) in vec3 fs_in_pos;
layout (location = 1) in vec3 fs_in_norm;
layout (location = 2) in vec2 fs_in_tex;
layout (location = 3) in flat uvec4 fs_in_index;
layout (location = 4) in mat3 fs_in_tbn;

//Outputs
layout (location = 0) out vec4 fs_out_pos;
layout (location = 1) out vec4 fs_out_color_metallic;
layout (location = 2) out vec4 fs_out_normal_roughness;
layout (location = 3) out uint fs_out_entity;
layout (location = 4) out vec4 fs_out_emissive_ao;

layout( push_constant ) uniform constants
{	
	uvec2 cameraBuffer;
	uvec2 transformBuffer;
	uvec2 instanceIndexAddressBuffer;
	uvec2 modelRenderIndicesBuffer;
	uvec2 modelBufferAddresses;
	uvec2 animationTransformBufferAddresses;
	uvec2 animationVertexBoneBufferAddresses;
	uvec2 shapeRenderIndicesBuffer;
	uvec2 shapeBufferAddresses;
	uvec2 shapeMaterialIndicesBuffer;
	uvec2 materialBuffer;
	uint renderMode;
	uint cameraIndex;
} PushConstants;

void main() 
{
	//TODO: CHECK FOR INVALID MATERIAL INDEX -> LOAD INVALID MATERIAL 
	uint materialIndex = fs_in_index.z;

	if(materialIndex == INVALID_IMAGE_INDEX)
		discard;

	MaterialBuffer materialBuffer = MaterialBuffer(PushConstants.materialBuffer);

	vec4 albedo = materialBuffer.materials[materialIndex].color;
	if(materialBuffer.materials[materialIndex].albedoIndex != uint(INVALID_IMAGE_INDEX))
		albedo *= sampleLoadedTexture2D(materialBuffer.materials[materialIndex].albedoIndex, LINEAR_ANISOTROPY_SAMPLER_ID, fs_in_tex);
	
	if(albedo.w < 0.1)
		discard;

	vec3 normal = normalize(fs_in_norm);
	if(materialBuffer.materials[materialIndex].normalIndex != uint(INVALID_IMAGE_INDEX))
	{
		normal = sampleLoadedTexture2D(materialBuffer.materials[materialIndex].normalIndex, LINEAR_ANISOTROPY_SAMPLER_ID, fs_in_tex).xyz;
		normal = normal * 2.0 - 1.0;   
        normal = normalize(fs_in_tbn * normal); 
		normal *= (0.5 - ((fs_in_index.w >> DIRECTX_NORMALS_BIT_INDEX) & 1)) * 2.0;
	}

	float metallic = materialBuffer.materials[materialIndex].metalness;
	if(materialBuffer.materials[materialIndex].metallicIndex != uint(INVALID_IMAGE_INDEX))
		metallic *= sampleLoadedTexture2D(materialBuffer.materials[materialIndex].metallicIndex, LINEAR_ANISOTROPY_SAMPLER_ID, fs_in_tex).x;

	float roughness = materialBuffer.materials[materialIndex].roughness;
	if(materialBuffer.materials[materialIndex].roughnessIndex != uint(INVALID_IMAGE_INDEX))
		roughness *= sampleLoadedTexture2D(materialBuffer.materials[materialIndex].roughnessIndex, LINEAR_ANISOTROPY_SAMPLER_ID, fs_in_tex).x;
	
	vec3 emission = materialBuffer.materials[materialIndex].emissiveColor.rgb * materialBuffer.materials[materialIndex].emissiveColor.a;
    if(materialBuffer.materials[materialIndex].emissiveIndex != uint(INVALID_IMAGE_INDEX))
        emission *= sampleLoadedTexture2D(materialBuffer.materials[materialIndex].emissiveIndex, LINEAR_ANISOTROPY_SAMPLER_ID, fs_in_tex).rgb;

    float ao = materialBuffer.materials[materialIndex].aoStrength; 
    if(materialBuffer.materials[materialIndex].ambientOcclusionIndex != uint(INVALID_IMAGE_INDEX))
        ao *= sampleLoadedTexture2D(materialBuffer.materials[materialIndex].ambientOcclusionIndex, LINEAR_ANISOTROPY_SAMPLER_ID, fs_in_tex).x;

	fs_out_pos = vec4(fs_in_pos, 1);
	fs_out_color_metallic = vec4(albedo.xyz, metallic);
	fs_out_normal_roughness = vec4(normal, roughness);
	fs_out_entity = fs_in_index.x;
	fs_out_emissive_ao = vec4(emission, ao);
}