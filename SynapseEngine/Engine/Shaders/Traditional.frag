#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(location = 0) in vec3 inNormal;
layout(location = 1) flat in uint inLodIndex;

layout(location = 0) out vec4 outFragColor;

void main() {
    vec3 color;

    if (inLodIndex == 0) {
        color = vec3(0.9, 0, 0);
    } else if (inLodIndex == 1) {
        color = vec3(0.6, 0, 0);
    } else if (inLodIndex == 2) {
        color = vec3(0.35, 0, 0);
    } else {
        color = vec3(0.15, 0, 0);
    }

    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float ndotl = max(dot(normalize(inNormal), lightDir), 0.0);
    vec3 finalColor = color;

    outFragColor = vec4(finalColor, 1.0);
}