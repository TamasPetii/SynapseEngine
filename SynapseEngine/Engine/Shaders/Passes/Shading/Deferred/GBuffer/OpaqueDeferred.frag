#version 460
#extension GL_GOOGLE_include_directive : require

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Common/Visibility.glsl"
#include "../../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../../Includes/Common/Texture.glsl"
#include "../../../../Includes/Common/Material.glsl"
#include "../../../../Includes/Utils/ColorMath.glsl"
#include "../../../../Includes/Utils/MaterialMath.glsl"

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inTangent;
layout(location = 2) in vec2 inUV;
layout(location = 3) in flat uvec3 inId; // (PackedEntity, Material, PartialPayload)

layout(location = 0) out vec4 outColorMetallic;
layout(location = 1) out vec4 outNormalRoughness;
layout(location = 2) out vec4 outEmissiveAo;
layout(location = 3) out uvec2 outId;

#include "../../../../Includes/PushConstants/TraditionalMeshletPassPC.glsl"

layout(push_constant) uniform PushConstants {
   TraditionalMeshletPassPC pc;
};

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);
    
    uint packedEntity = inId.x;
    uint materialId   = inId.y;
    uint partial      = inId.z;

    uint pipelineFlag = UNPACK_VISIBILITY_PIPELINE(packedEntity);
    uint finalPayload = pipelineFlag == VIS_PIPELINE_MESH_SHADER 
                                      ? FINALIZE_VIS_MS(partial, gl_PrimitiveID)
                                      : FINALIZE_VIS_TRADITIONAL(partial, gl_PrimitiveID);

    // 1. Fetch Material
    Material mat = GET_MATERIAL(ctx.materialBufferAddr, materialId);
    vec2 finalUV = inUV * mat.uvScale;

    // 2. Evaluate Albedo & Alpha
    vec4 albedoAlpha = EvaluateAlbedoAlpha(ctx.textureMetadataBufferAddr, mat, finalUV);
    if (albedoAlpha.a < ctx.alphaLimitDiscard) {
        discard;
    }

    // 3. Evaluate Normals & TBN
    vec3 finalNormal = EvaluateNormal(ctx.textureMetadataBufferAddr, mat, finalUV, inNormal, inTangent);

    // 4. Evaluate Metalness & Roughness
    vec2 metalRough = EvaluateMetallicRoughness(ctx.textureMetadataBufferAddr, mat, finalUV);
    float finalMetalness = metalRough.x;
    float finalRoughness = clamp(metalRough.y, 0.04, 1.0);

    // 5. Evaluate Emissive
    vec3 finalEmissive = EvaluateEmissive(ctx.textureMetadataBufferAddr, mat, finalUV);

    // 6. Evaluate Ambient Occlusion
    float finalAo = EvaluateAO(ctx.textureMetadataBufferAddr, mat, finalUV);

    // 7. Write Standard Outputs
    outColorMetallic   = vec4(albedoAlpha.rgb, finalMetalness);
    outNormalRoughness = vec4(finalNormal, finalRoughness);
    outEmissiveAo      = vec4(finalEmissive, finalAo);
    outId              = uvec2(packedEntity, finalPayload);
}