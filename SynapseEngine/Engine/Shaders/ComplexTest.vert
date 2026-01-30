#version 460

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 viewProj;
} camera;

layout(push_constant) uniform PushConsts {
    mat4 model;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out VS_OUT {
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
} vs_out;

void main() {
    vec4 worldPos = pc.model * vec4(inPosition, 1.0);
    vs_out.worldPos = worldPos.xyz;
    vs_out.texCoord = inTexCoord;
    vs_out.normal = mat3(transpose(inverse(pc.model))) * inNormal;
    gl_Position = camera.viewProj * worldPos;
}