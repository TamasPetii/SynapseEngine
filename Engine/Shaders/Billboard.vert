#version 460

#include "Common/Position.glsl"
#include "Common/Render/InstanceIndex.glsl"

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
	uint index = InstanceIndexBuffer(PushConstants.instanceBufferAddress).indices[gl_VertexIndex];
	gl_Position = PositionBuffer(PushConstants.positionBufferAddress).positions[index];
	//position.w is gonna be entity index!!!
}