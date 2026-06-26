#version 460

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Utils/LightMath.glsl"
#include "../../../../Includes/Common/FrameGlobalContext.glsl"

layout(set = 2, binding = 0) uniform sampler2D colorMetallicTexture;
layout(set = 2, binding = 1) uniform sampler2D emissiveAoTexture;
layout(set = 2, binding = 2) uniform sampler2D ssaoTexture;

#include "../../../../Includes/PushConstants/DeferredEmissiveAoPC.glsl"

layout(push_constant) uniform PushConstants {
    DeferredEmissiveAoPC pc;
};

void main()
{
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    vec3 albedo = texture(colorMetallicTexture, inUV).xyz;
    vec4 emissiveAo = texture(emissiveAoTexture, inUV);
    
    vec3 emissive = emissiveAo.rgb;
    float ao = emissiveAo.a;

    float ssao = 1.0;
    if (ctx.enableSsao == 1)
        ssao = texture(ssaoTexture, inUV).r;  

    float finalAo = ao * ssao;

    vec3 ambientResult = SimulateAmbientLight(albedo, finalAo, ctx.ambientStrength);
    vec3 emissiveResult = SimulateBloom(emissive, 1.0, ctx.emissiveStrength);

    outColor = vec4(ambientResult + emissiveResult, 1.0);
}