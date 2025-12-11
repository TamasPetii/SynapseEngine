#version 460

#include "Common/Plane.glsl"

layout(location = 0) out vec2 vs_out_tex;

void main() 
{
	gl_Position = positions[gl_VertexIndex];
	vs_out_tex	= texCoords[gl_VertexIndex];
}