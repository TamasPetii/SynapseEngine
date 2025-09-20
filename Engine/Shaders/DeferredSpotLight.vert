#version 460

#include "Common/Index.glsl"
#include "Common/Camera.glsl"
#include "Common/Vertex.glsl"
#include "Common/SimpleTransform.glsl"
#include "Common/Render/InstanceIndex.glsl"

layout(location = 0) out flat uint vs_out_id;

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

void main() 
{
	uint lightIndex = InstanceIndexBuffer(PushConstants.instanceBufferAddress).indices[gl_InstanceIndex];

	uint vertexIndex = IndexBuffer(PushConstants.indexBufferAddress).indices[gl_VertexIndex];
	Vertex v = VertexBuffer(PushConstants.vertexBufferAddress).vertices[vertexIndex];

	vec4 worldPosition = SimpleTransformBuffer(PushConstants.transformBufferAddress).transforms[lightIndex] * vec4(v.position, 1.0);
	gl_Position = CameraBuffer(PushConstants.cameraBuffer).cameras[PushConstants.cameraIndex].viewProj * worldPosition;

	vs_out_id = lightIndex;
}