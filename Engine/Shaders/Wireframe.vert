#version 460
#extension GL_EXT_buffer_reference_uvec2 : require

#include "Common/Index.glsl"
#include "Common/Vertex.glsl"
#include "Common/Camera.glsl"
#include "Common/SimpleTransform.glsl"

layout( push_constant ) uniform constants
{	
	uvec2 cameraBufferAddress;
	uvec2 transformBufferAddress;
	uvec2 vertexBufferAddress;
	uvec2 indexBufferAddress;
	vec4 color;
	vec3 padding;
	uint cameraIndex;
} PushConstants;

void main() 
{
	uint vertexIndex = IndexBuffer(PushConstants.indexBufferAddress).indices[gl_VertexIndex];
	Vertex v = VertexBuffer(PushConstants.vertexBufferAddress).vertices[vertexIndex];
	gl_Position = CameraBuffer(PushConstants.cameraBufferAddress).cameras[PushConstants.cameraIndex].viewProjVulkan * SimpleTransformBuffer(PushConstants.transformBufferAddress).transforms[gl_InstanceIndex] * vec4(v.position + v.normal * 0.001, 1.0);
}