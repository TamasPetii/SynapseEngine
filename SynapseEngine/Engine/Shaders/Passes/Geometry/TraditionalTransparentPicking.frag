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
layout(location = 3) in flat uvec4 inId; //(EntityID, MaterialID, MeshIndex, LodIndex) 

layout(location = 0) out uint outEntityId;
layout(location = 1) out vec4 outDebugTopologyPipeline;
layout(location = 2) out vec4 outDebugMeshletLod;
layout(location = 3) out vec4 outDebugMaterialUv;

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

    // 2. Albedo & Alpha
    vec4 finalColor = mat.color;
    if (HAS_ALBEDO_TEX(mat)) {
        finalColor *= SampleTexture2D(mat.albedoTexture, SAMPLER_LINEAR_ANISO, finalUV);
    }

    // Alpha test for picking
    if (finalColor.a < 0.05) {
        discard;
    }

    // 3. Write ID output
    outEntityId = entityId;

    // 4. Write Debug outputs
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