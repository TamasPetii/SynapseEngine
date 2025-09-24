#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct Camera
{
	mat4 view;
	mat4 viewInv;
	mat4 proj;
	mat4 projInv;
	mat4 viewProj;
	mat4 viewProjInv;
	vec4 eye;

};

layout(buffer_reference, std430) readonly buffer CameraBuffer { 
	Camera cameras[];
};