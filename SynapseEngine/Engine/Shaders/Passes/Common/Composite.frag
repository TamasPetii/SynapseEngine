#version 460
layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D texColor;

void main() 
{
    outColor = texture(texColor, inUV);
}