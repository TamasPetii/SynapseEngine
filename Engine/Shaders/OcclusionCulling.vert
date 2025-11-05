#version 460

#include "Common/Camera.glsl"
#include "Common/Index.glsl"
#include "Common/Vertex.glsl"
#include "Common/SimpleTransform.glsl"
#include "Common/Render/InstanceIndex.glsl"

layout(location = 0) out flat uint vs_out_id;

layout( push_constant ) uniform constants
{	
	uvec2 cameraBuffer;
	uvec2 transformBufferAddress;
	uvec2 instanceBufferAddress;
	uvec2 vertexBufferAddress;
	uvec2 indexBufferAddress;
	uvec2 occlusionBufferAddress;
	vec3 padding;
	uint cameraIndex;
} PushConstants;

void main() 
{
	uint objectIndex = InstanceIndexBuffer(PushConstants.instanceBufferAddress).indices[gl_InstanceIndex];

	uint vertexIndex = IndexBuffer(PushConstants.indexBufferAddress).indices[gl_VertexIndex];
	Vertex v = VertexBuffer(PushConstants.vertexBufferAddress).vertices[vertexIndex];

	vec4 worldPosition = SimpleTransformBuffer(PushConstants.transformBufferAddress).transforms[objectIndex] * vec4(v.position, 1.0);
	gl_Position = CameraBuffer(PushConstants.cameraBuffer).cameras[PushConstants.cameraIndex].viewProjVulkan * worldPosition;

	vs_out_id = gl_InstanceIndex;
}