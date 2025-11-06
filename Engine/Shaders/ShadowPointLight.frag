#version 460
#include "Common/Light/PointLight.glsl"

layout(location = 0) in vec4 fs_in_pos;

layout( push_constant ) uniform constants
{	
	uvec2 transformBuffer;
	uvec2 instanceIndexAddressBuffer;
	uvec2 modelRenderIndicesBuffer;
	uvec2 modelBufferAddresses;
	uvec2 animationTransformBufferAddresses;
	uvec2 animationVertexBoneBufferAddresses;
	uvec2 shapeRenderIndicesBuffer;
	uvec2 shapeBufferAddresses;
    uvec2 pointLightBufferAddress;
	uint renderMode;
	uint pointLightIndex;
} PushConstants;

void main()
{
    float lightDistance = length(fs_in_pos.xyz - PointLightBuffer(PushConstants.pointLightBufferAddress).lights[PushConstants.pointLightIndex].position); 
    float lightDistanceLinearNorm = lightDistance / PointLightBuffer(PushConstants.pointLightBufferAddress).lights[PushConstants.pointLightIndex].radius;
    gl_FragDepth = lightDistanceLinearNorm;
}  