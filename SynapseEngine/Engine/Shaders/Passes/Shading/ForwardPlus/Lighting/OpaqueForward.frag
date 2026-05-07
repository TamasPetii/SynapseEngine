#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../../Includes/Common/Texture.glsl"
#include "../../../../Includes/Common/Material.glsl"
#include "../../../../Includes/Common/Camera.glsl"
#include "../../../../Includes/Common/Cluster.glsl"
#include "../../../../Includes/Common/DirectionLight.glsl"
#include "../../../../Includes/Common/PointLight.glsl"
#include "../../../../Includes/Common/SpotLight.glsl"
#include "../../../../Includes/Utils/ColorMath.glsl"
#include "../../../../Includes/Utils/PbrMath.glsl"
#include "../../../../Includes/Utils/ClusterMath.glsl"
#include "../../../../Includes/Utils/DepthMath.glsl"
#include "../../../../Includes/Utils/LightMath.glsl"
#include "../../../../Includes/Utils/MaterialMath.glsl"

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inTangent;
layout(location = 2) in vec2 inUV;
layout(location = 3) in flat uvec4 inId; // (EntityID, MaterialID, MeshletIndex, LodIndex) 

layout(location = 0) out vec4 outColor;

#include "../../../../Includes/PushConstants/TraditionalMeshletPassPC.glsl"

layout(push_constant) uniform PushConstants {
   TraditionalMeshletPassPC pc;
};

void main() {
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    uint entityId = inId.x;
    uint materialId = inId.y;

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

    uint cameraDenseIndex = GET_SPARSE_INDEX(ctx.cameraSparseMapBufferAddr, ctx.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(ctx.cameraBufferAddr, cameraDenseIndex);

    vec2 screenUV = gl_FragCoord.xy / vec2(ctx.screenWidth, ctx.screenHeight);
    float fragDepth = gl_FragCoord.z;
    vec3 worldPos = ReconstructWorldPosition(screenUV, fragDepth, camera.viewProjVulkanInv);

    vec4 viewPos = camera.view * vec4(worldPos, 1.0);
    float viewDepth = abs(viewPos.z);
    vec3 viewDir = normalize(camera.eye.xyz - worldPos);

    uint tileX = uint(gl_FragCoord.x) / ctx.tileSize;
    uint tileY = uint(gl_FragCoord.y) / ctx.tileSize;
    uint tileIndex = tileY * ctx.tileCountX + tileX;

    TileData tile = GET_TILE_DATA(ctx.forwardPlusTileGridListBufferAddr, tileIndex);
    uint sliceIdx = GetClusterSliceIndex(viewDepth, tile.minZ, tile.maxZ, tile.sliceCount);
    
    uint clusterIndex = tile.clusterBaseOffset + sliceIdx;
    ClusterData cluster = GET_CLUSTER_DATA(ctx.forwardPlusClusterListBufferAddr, clusterIndex);

    // 7. Simulate Lighting
    vec3 totalRadiance = vec3(0.0);

    for(uint i = 0; i < ctx.directionLightCount; ++i) {
        uint lightDenseIndex = GET_VISIBLE_DIRECTION_LIGHT(ctx.directionLightVisibleIndexBufferAddr, i); 
        totalRadiance += SimulateDirectionalLight(ctx.directionLightDataBufferAddr, lightDenseIndex, albedoAlpha.rgb, finalNormal, viewDir, finalRoughness, finalMetalness);
    }

    for (uint i = 0; i < cluster.pointLightCount; ++i) {
        uint globalLightIndex = cluster.pointLightOffset + i;
        uint lightEntityIndex = GET_LIGHT_INDEX(ctx.forwardPlusPointLightIndexListBufferAddr, globalLightIndex);
        uint lightDenseIndex = GET_SPARSE_INDEX(ctx.pointLightSparseMapBufferAddr, lightEntityIndex);
        
        totalRadiance += SimulatePointLight(ctx.pointLightDataBufferAddr, lightDenseIndex, inWorldPos, albedoAlpha.rgb, finalNormal, viewDir, finalRoughness, finalMetalness);
    }

    for (uint i = 0; i < cluster.spotLightCount; ++i) {
        uint globalLightIndex = cluster.spotLightOffset + i;
        uint lightEntityIndex = GET_LIGHT_INDEX(ctx.forwardPlusSpotLightIndexListBufferAddr, globalLightIndex);
        uint lightDenseIndex = GET_SPARSE_INDEX(ctx.spotLightSparseMapBufferAddr, lightEntityIndex);
        
        totalRadiance += SimulateSpotLight(ctx.spotLightDataBufferAddr, lightDenseIndex, inWorldPos, albedoAlpha.rgb, finalNormal, viewDir, finalRoughness, finalMetalness);
    }

    //Ambient
    totalRadiance += SimulateAmbientLight(albedoAlpha.rgb, finalAo, ctx.ambientStrength);

    //Bloom Radiance
    totalRadiance += SimulateBloom(finalEmissive, 1.0, ctx.emissiveStrength);

    outColor = vec4(totalRadiance, 1.0);
}