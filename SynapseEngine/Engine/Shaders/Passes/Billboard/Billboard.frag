#version 460
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 inUV;
layout(location = 1) in flat uint inEntityId;

layout(location = 0) out vec4 outColor;
layout(location = 1) out uint outEntityId;

layout(set = 2, binding = 0) uniform sampler2D iconTexture;

void main() {
    vec4 texColor = texture(iconTexture, inUV);
    if (texColor.a < 0.1) discard;

    outColor = texColor;
    outEntityId = inEntityId;
}