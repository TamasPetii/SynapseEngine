#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

struct TriangleData {
    vec4 normalModifier;
};

layout(std430, set = 1, binding = 0) readonly buffer ObjectDataBuffer {
    TriangleData data[];
} objectData;

layout(push_constant) uniform PushConsts {
    mat4 model;
} pc;

layout(location = 0) in VS_OUT {
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
} gs_in[];

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outColorMod;

void main() {
    vec3 extraMod = objectData.data[0].normalModifier.xyz;

    for(int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        outTexCoord = gs_in[i].texCoord;
        outNormal = normalize(gs_in[i].normal + extraMod);
        outColorMod = extraMod;
        EmitVertex();
    }
    EndPrimitive();
}