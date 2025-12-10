#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct Camera
{
	mat4 view;
	mat4 viewInv;
	mat4 proj;
	mat4 projInv;
	mat4 projVulkan;
	mat4 projVulkanInv;
	mat4 viewProj;
	mat4 viewProjInv;
	mat4 viewProjVulkan;
	mat4 viewProjVulkanInv;
	vec4 eye; //xyz: position, w = padding
	vec4 params; // x: near, y: far, z: fov, w: aspect
};

layout(buffer_reference, std430) readonly buffer CameraBuffer { 
	Camera cameras[];
};

struct CameraFrustum
{
	//vec3 normal + float distance
	vec4 near;
	vec4 right;
	vec4 left;
	vec4 top;
	vec4 bottom;
	vec4 far;
};

layout(buffer_reference, std430) readonly buffer CameraFrustumBuffer { 
	CameraFrustum frustums[];
};