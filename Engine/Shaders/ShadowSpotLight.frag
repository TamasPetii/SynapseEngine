#version 460
#include "Common/Light/SpotLight.glsl"
#include "Common/Utility/LinearDepth.glsl"

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
    uvec2 spotLightBufferAddress; 
    uvec2 spotLightShadowBufferAddress;
    uint renderMode;
    uint spotLightIndex;
} PushConstants;

void main()
{
    uint shadowIndex = SpotLightBuffer(PushConstants.spotLightBufferAddress).lights[PushConstants.spotLightIndex].shadowIndex;

    float near = SpotLightShadowBuffer(PushConstants.spotLightShadowBufferAddress).lightShadows[shadowIndex].nearPlane;
    float far = SpotLightShadowBuffer(PushConstants.spotLightShadowBufferAddress).lightShadows[shadowIndex].farPlane;

    gl_FragDepth = ConvertDepthToLinearNormalized(gl_FragCoord.z, near, far);
}