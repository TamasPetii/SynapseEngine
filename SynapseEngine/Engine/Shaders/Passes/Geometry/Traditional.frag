#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "../../Includes/Core.glsl"
#include "../../Includes/Common/Material.glsl"
#include "../../Includes/Common/Texture.glsl"
#include "../../Includes/Utils/ColorMath.glsl"

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inTangent;
layout(location = 2) in vec2 inUV;
layout(location = 3) in flat uvec4 inId; // (EntityID, MaterialID, MeshIndex, LodIndex) 

layout(location = 0) out vec4 outColorMetallic;
layout(location = 1) out vec4 outNormalRoughness;
layout(location = 2) out vec4 outEmissiveAo;
layout(location = 3) out uint outEntityId;
layout(location = 4) out vec4 outDebugTopologyPipeline;
layout(location = 5) out vec4 outDebugMeshletLod;
layout(location = 6) out vec4 outDebugMaterialUv;

#include "../../Includes/PushConstants/TraditionalPassPC.glsl"

void main() 
{ 
    uint entityId = inId.x;
    uint materialId = inId.y;
    uint meshIndex = inId.z;
    uint lodIndex = inId.w;

    // 1. Fetch Material
    Material mat = GET_MATERIAL(pc.materialBuffer, materialId);
    vec2 finalUV = inUV * mat.uvScale;

    // 2. Evaluate Albedo & Alpha
    vec4 finalColor = mat.color;
    if (HAS_ALBEDO_TEX(mat)) {
        finalColor *= SampleTexture2D(mat.albedoTexture, SAMPLER_LINEAR_ANISO, finalUV);
    }

    if (finalColor.a < 0.05) {
        discard;
    }

    // 3. Evaluate Normals & TBN
    vec3 normal = normalize(inNormal);
    vec3 finalNormal = normal;
    
    if (HAS_NORMAL_TEX(mat)) {
        vec3 tangent = normalize(inTangent.xyz);
        tangent = normalize(tangent - finalNormal * dot(finalNormal, tangent));
        vec3 bitangent = cross(finalNormal, tangent) * inTangent.w;
        mat3 TBN = mat3(tangent, bitangent, finalNormal);

        vec3 normalMapSample = SampleTexture2D(mat.normalTexture, SAMPLER_LINEAR_ANISO, finalUV).rgb;
        vec3 tangentSpaceNormal = normalMapSample * 2.0 - 1.0;
        finalNormal = normalize(TBN * tangentSpaceNormal);
    }

    // 4. Evaluate Metalness & Roughness
    float finalMetalness = mat.metalness;
    float finalRoughness = mat.roughness;

    if (HAS_METALNESS_TEX(mat)) {
        finalMetalness *= SampleTexture2D(mat.metalnessTexture, SAMPLER_LINEAR_ANISO, finalUV).r;
    }

    if (HAS_ROUGHNESS_TEX(mat)) {
        finalRoughness *= SampleTexture2D(mat.roughnessTexture, SAMPLER_LINEAR_ANISO, finalUV).r;
    }

    if (HAS_METALLIC_ROUGHNESS_TEX(mat)) {
        vec4 mrSample = SampleTexture2D(mat.metallicRoughnessTexture, SAMPLER_LINEAR_ANISO, finalUV);
        finalRoughness *= mrSample.g;
        finalMetalness *= mrSample.b;
    }

    // 5. Evaluate Emissive
    vec3 finalEmissive = mat.emissiveColor * mat.emissiveIntensity;
    if (HAS_EMISSIVE_TEX(mat)) {
        finalEmissive *= SampleTexture2D(mat.emissiveTexture, SAMPLER_LINEAR_ANISO, finalUV).rgb;
    }

    // 6. Evaluate Ambient Occlusion
    float finalAo = mat.aoStrength;
    if (HAS_AO_TEX(mat)) {
        finalAo *= SampleTexture2D(mat.ambientOcclusionTexture, SAMPLER_LINEAR_ANISO, finalUV).r;
    }

    // 7. Write Standard Outputs
    outColorMetallic   = vec4(finalColor.rgb, finalMetalness);
    outNormalRoughness = vec4(finalNormal, finalRoughness);
    outEmissiveAo      = vec4(finalEmissive, finalAo);
    outEntityId        = entityId;

    // 8. Write Debug Outputs
    vec3 traditionalStructureColor = getDebugColor(entityId, meshIndex, lodIndex, gl_FragCoord.xy);
    outDebugTopologyPipeline = vec4(traditionalStructureColor, 0.25);

    float grayscaleLOD = 1.0 - (float(lodIndex) / 3.0);
    outDebugMeshletLod = vec4(0.0, 0.0, 0.0, grayscaleLOD);

    vec2 baseMatColor = MAT_TYPE_COLORS[pc.materialRenderType % 4];
    float idHashModifier = (float(hash(materialId) & 0xFF) / 255.0) * 0.4 - 0.2;
    vec2 finalMatColor = SATURATE(baseMatColor + idHashModifier);
    
    vec2 debugUV = fract(finalUV);
    outDebugMaterialUv = vec4(finalMatColor.x, finalMatColor.y, debugUV.x, debugUV.y);
}