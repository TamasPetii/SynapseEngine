#version 460
#include "Common/Light/PointLight.glsl"

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;
layout(location = 0) out vec4 gs_out_pos;

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
    uvec2 pointLightShadowBufferAddress;
	uint renderMode;
	uint pointLightIndex;
} PushConstants;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i)
        {
            gs_out_pos = gl_in[i].gl_Position;
            uint shadowIndex = PointLightBuffer(PushConstants.pointLightBufferAddress).lights[PushConstants.pointLightIndex].shadowIndex;
            gl_Position = PointLightShadowBuffer(PushConstants.pointLightShadowBufferAddress).lightShadows[shadowIndex].viewProj[face] * gs_out_pos;
            EmitVertex();
        }    
        EndPrimitive();
    }
}  