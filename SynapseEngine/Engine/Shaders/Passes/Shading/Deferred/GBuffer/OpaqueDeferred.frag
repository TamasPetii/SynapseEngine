#version 460
#extension GL_GOOGLE_include_directive : require

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../../Includes/Common/Texture.glsl"
#include "../../../../Includes/Common/Material.glsl"
#include "../../../../Includes/Utils/ColorMath.glsl"
#include "../../../../Includes/Utils/LightMath.glsl"
#include "../../../../Includes/Utils/MaterialMath.glsl"

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inTangent;
layout(location = 2) in vec2 inUV;
layout(location = 3) in flat uvec4 inId; // (EntityID, MaterialID, Mesh/MeshletIndex, LodIndex) 

layout(location = 0) out vec4 outColorMetallic;
layout(location = 1) out vec4 outNormalRoughness;
layout(location = 2) out vec4 outEmissiveAo;
layout(location = 3) out uint outEntityId;

#include "../../../../Includes/PushConstants/TraditionalMeshletPassPC.glsl"

layout(push_constant) uniform PushConstants {
   TraditioMeshletPassPC pc;
};

void main() {
    uint entityId = inId.x;
    uint materialId = inId.y;
    uint meshletIndex = inId.z;
    uint lodIndex = inId.w;

    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    // 1. Fetch Material
    Material mat = GET_MATERIAL(ctx.materialBufferAddr, materialId);
    vec2 finalUV = inUV * mat.uvScale;

    // 2. Evaluate Albedo & Alpha
    vec4 albedoAlpha = EvaluateAlbedoAlpha(mat, finalUV);
    if (albedoAlpha.a < ctx.alphaLimitDiscard) {
        discard;
    }

    // 3. Evaluate Normals & TBN
    vec3 finalNormal = EvaluateNormal(mat, finalUV, inNormal, inTangent);

    // 4. Evaluate Metalness & Roughness
    vec2 metalRough = EvaluateMetallicRoughness(mat, finalUV);
    float finalMetalness = metalRough.x;
    float finalRoughness = clamp(metalRough.y, 0.04, 1.0);

    // 5. Evaluate Emissive
    vec3 finalEmissive = EvaluateEmissive(mat, finalUV);

    // 6. Evaluate Ambient Occlusion
    float finalAo = EvaluateAO(mat, finalUV);

    // 7. Write Standard Outputs
    outColorMetallic   = vec4(albedoAlpha.rgb, finalMetalness);
    outNormalRoughness = vec4(finalNormal, finalRoughness);
    outEmissiveAo      = vec4(finalEmissive, finalAo);
    outEntityId        = entityId;
}