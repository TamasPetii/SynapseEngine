#version 460

#include "Common/Camera.glsl"
#include "Common/Render/ColliderDebug.glsl"
#include "Common/Render/InstanceIndex.glsl"

layout (location = 0) out flat uint vs_out_index;

layout(push_constant) uniform constants
{
	vec4 color;
    uvec2 colliderDebugBuffer;
    uvec2 instanceIndexBuffer;
} PushConstants;

void main() 
{
    if (gl_VertexIndex > 4) 
    {
        gl_Position = vec4(uintBitsToFloat(0x7FC00000)); 
        return;
    }
    
    uint index = InstanceIndexBuffer(PushConstants.instanceIndexBuffer).indices[gl_InstanceIndex];
    uint debugIndex = gl_InstanceIndex; 

    vec4 aabb = ColliderDebugBuffer(PushConstants.colliderDebugBuffer).data[debugIndex].projectedAABB;

    vec2 pos;
    int vertexID = gl_VertexIndex % 5;

    if (vertexID == 0) pos = vec2(aabb.x, aabb.y);      // MinX, MinY (Bal-Fent)
    else if (vertexID == 1) pos = vec2(aabb.z, aabb.y); // MaxX, MinY (Jobb-Fent)
    else if (vertexID == 2) pos = vec2(aabb.z, aabb.w); // MaxX, MaxY (Jobb-Lent)
    else if (vertexID == 3) pos = vec2(aabb.x, aabb.w); // MinX, MaxY (Bal-Lent)
    else if (vertexID == 4) pos = vec2(aabb.x, aabb.y); // Vissza az elejére

    vec2 ndc = vec2(
        pos.x * 2.0 - 1.0,
        1.0 - pos.y * 2.0
    );

    gl_Position = vec4(ndc, 0.0, 1.0);

    vs_out_index = debugIndex;
}