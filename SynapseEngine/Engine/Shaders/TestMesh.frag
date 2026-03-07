#version 460
#extension GL_EXT_mesh_shader : require

layout(location = 0) in vec3 inColor;
layout(location = 0) out vec4 outFragColor;

void main() {
    outFragColor = vec4(inColor, 1.0);
}