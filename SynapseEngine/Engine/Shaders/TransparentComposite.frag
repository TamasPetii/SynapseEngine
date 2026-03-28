#version 460

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D accumTex;
layout(set = 2, binding = 1) uniform sampler2D revealTex;

void main() {
    float revealage = texture(revealTex, inUV).r;
    
    if (revealage == 1.0) 
        discard;

    vec4 accum = texture(accumTex, inUV);
    vec3 averageColor = accum.rgb / max(accum.a, 1e-5);

    outColor = vec4(averageColor * (1.0 - revealage), revealage);
}