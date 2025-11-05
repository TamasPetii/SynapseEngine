#version 460

#include "Common/Camera.glsl"

layout(points) in;
layout(triangle_strip, max_vertices = 6) out;

layout( push_constant ) uniform constants
{	
	uvec2 cameraBuffer;
	uvec2 positionBufferAddress;
	uvec2 instanceBufferAddress;
	uint cameraIndex;
	uint iconIndex;
} PushConstants;

layout(location = 0) out vec2 gs_out_tex;
layout(location = 1) out flat uint gs_out_id;

void main()
{	
	CameraBuffer cameraBuffer = CameraBuffer(PushConstants.cameraBuffer);
	const float iconSize = 1.0;

	vec3 to_eye = cameraBuffer.cameras[PushConstants.cameraIndex].eye.xyz - gl_in[0].gl_Position.xyz;
	vec3 to_up  = vec3(0,1,0);
	vec3 to_right = normalize(cross(to_eye, to_up));

	to_up *= iconSize;
	to_right *= iconSize;

	//Bottom Left
	gl_Position = cameraBuffer.cameras[PushConstants.cameraIndex].viewProjVulkan * vec4(gl_in[0].gl_Position.xyz - to_up - to_right, 1);
	gs_out_tex = vec2(0, 0);
	gs_out_id = uint(gl_in[0].gl_Position.w);
	EmitVertex();

	//Top Left
	gl_Position = cameraBuffer.cameras[PushConstants.cameraIndex].viewProjVulkan * vec4(gl_in[0].gl_Position.xyz + to_up - to_right, 1);
	gs_out_tex = vec2(0, 1);
	gs_out_id = uint(gl_in[0].gl_Position.w);
	EmitVertex();

	//Bottom Right
	gl_Position = cameraBuffer.cameras[PushConstants.cameraIndex].viewProjVulkan * vec4(gl_in[0].gl_Position.xyz - to_up + to_right, 1);
	gs_out_tex = vec2(1, 0);
	gs_out_id = uint(gl_in[0].gl_Position.w);
	EmitVertex();

	//Top Right
	gl_Position = cameraBuffer.cameras[PushConstants.cameraIndex].viewProjVulkan * vec4(gl_in[0].gl_Position.xyz + to_up + to_right, 1);
	gs_out_tex = vec2(1, 1);
	gs_out_id = uint(gl_in[0].gl_Position.w);
	EmitVertex();

	EndPrimitive();
}