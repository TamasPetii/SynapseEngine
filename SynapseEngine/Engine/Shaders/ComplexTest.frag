#version 450

layout(set = 2, binding = 0) uniform sampler2D myTexture;
layout(set = 2, binding = 1) uniform sampler2D myNormalMap;

layout(push_constant) uniform PushConsts {
    layout(offset = 64) vec4 colorTint;
} pc;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColorMod;

layout(location = 0) out vec4 outFragColor;

void main() {
    vec4 texColor = texture(myTexture, inTexCoord);
    outFragColor = texColor * pc.colorTint * vec4(inColorMod, 1.0);
}