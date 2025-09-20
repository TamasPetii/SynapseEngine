#version 460
#extension GL_EXT_buffer_reference_uvec2 : require

#include "Common/Images.glsl"

layout(location = 0) in vec2 fs_in_tex;
layout(location = 1) in flat uint fs_in_id;
layout(location = 0) out vec4 fs_out_col;
layout(location = 1) out uint fs_out_id;

layout( push_constant ) uniform constants
{	
	uvec2 cameraBuffer;
	uvec2 positionBufferAddress;
	uvec2 instanceBufferAddress;
	uint cameraIndex;
	uint iconIndex;
} PushConstants;

void main()
{
	//vec4 color = sampleTexture2D(PushConstants.iconIndex, LINEAR_ANISOTROPY_SAMPLER_ID, fs_in_tex);

	vec4 color = vec4(1,0,0,1);

	if(color.w < 0.05)
		discard;

	fs_out_col = color;
	fs_out_id = fs_in_id;
}