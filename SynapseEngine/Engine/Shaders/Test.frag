#version 460
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(location = 0) in vec3 inNormal;
layout(location = 0) out vec4 outFragColor;

void main() {
    vec3 color = inNormal * 0.5 + 0.5;
    outFragColor = vec4(color, 1.0);
}