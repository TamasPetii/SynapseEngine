#version 460
#include "Common/Render/ColliderDebug.glsl"

layout(location = 0) in flat uint fs_in_index;
layout(location = 0) out vec4 fs_out_col;

layout(push_constant) uniform constants
{
	vec4 color;
    uvec2 colliderDebugBuffer;
    uvec2 instanceIndexBuffer;
} PushConstants;

void main()
{
    float depth = ColliderDebugBuffer(PushConstants.colliderDebugBuffer).data[fs_in_index].projectedLinearDepth;
    fs_out_col = vec4(depth > 0.98 ? vec3(1, 1, 1) : PushConstants.color.xyz, 1);
}