#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference_uvec2 : require

struct ColliderDebug
{
	vec4 projectedAABB;
	float projectedLinearDepth;
	uint paddingA;
	uint paddingB;
	uint paddingC;
};

layout(buffer_reference, std430) readonly buffer ColliderDebugBuffer {
	ColliderDebug data[];
};

