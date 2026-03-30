#version 460

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D colorMetallicTexture;
layout(set = 2, binding = 1) uniform sampler2D emissiveAoTexture;

layout(push_constant) uniform PushConstants {
    float ambientStrength;
    float emissiveStrength;
} pc;

void main()
{
    vec3 albedo = texture(colorMetallicTexture, inUV).xyz;
    vec4 emissiveAo = texture(emissiveAoTexture, inUV);
    
    vec3 emissive = emissiveAo.rgb;
    float ao = emissiveAo.a;

    vec3 ambientResult = albedo * pc.ambientStrength * ao;
    vec3 emissiveResult = emissive * pc.emissiveStrength;

    outColor = vec4(ambientResult + emissiveResult, 1.0);
}