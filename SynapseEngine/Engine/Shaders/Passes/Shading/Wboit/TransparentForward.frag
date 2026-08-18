// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

#include "../../../Includes/Core.glsl"
#include "../../../Includes/Common/Cluster.glsl"
#include "../../../Includes/Common/Camera.glsl"
#include "../../../Includes/Common/FrameGlobalContext.glsl"
#include "../../../Includes/Common/Material.glsl"
#include "../../../Includes/Common/Texture.glsl"
#include "../../../Includes/Utils/WboitMath.glsl"
#include "../../../Includes/Utils/DepthMath.glsl"
#include "../../../Includes/Utils/MaterialMath.glsl"
#include "../../../Includes/Utils/ClusterMath.glsl"
#include "../../../Includes/Utils/LightMath.glsl"
#include "../../../Includes/Utils/ShadowMath.glsl"

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inTangent;
layout(location = 2) in vec2 inUV;
layout(location = 3) in flat uvec3 inId; // (PackedEntity, Material, PartialPayload)

layout(location = 0) out vec4 outAccum;
layout(location = 1) out float outReveal;

layout(set = 2, binding = 2) uniform sampler2DShadow dirLightShadowAtlas;
layout(set = 2, binding = 3) uniform sampler2DShadow pointLightShadowAtlas;
layout(set = 2, binding = 4) uniform sampler2DShadow spotLightShadowAtlas;

#include "../../../Includes/PushConstants/TraditionalMeshletPassPC.glsl"

layout(push_constant) uniform PushConstants {
   TraditionalMeshletPassPC pc;
};

void main() 
{ 
    FrameGlobalContext ctx = GET_FRAME_CONTEXT(pc.frameGlobalContextBufferAddr);

    uint materialId = inId.y;
    
    // 1. Fetch Material
    Material mat = GET_MATERIAL(ctx.materialBufferAddr, materialId);
    vec2 finalUV = inUV * mat.uvScale;

    // 2. Evaluate Albedo & Alpha
    vec4 albedoAlpha = EvaluateAlbedoAlpha(ctx.textureMetadataBufferAddr, mat, finalUV);
    if (IS_ALPHA_TESTED(mat) && albedoAlpha.a < ctx.alphaLimitDiscard) {
        discard;
    }

    // 3. Evaluate Normals & TBN
    bool frontFacing = IS_DOUBLE_SIDED(mat) ? gl_FrontFacing : true;
    vec3 finalNormal = EvaluateNormal(ctx.textureMetadataBufferAddr, mat, finalUV, inNormal, inTangent, frontFacing);

    // 4. Evaluate Metalness & Roughness
    vec2 metalRough = EvaluateMetallicRoughness(ctx.textureMetadataBufferAddr, mat, finalUV);
    float finalMetalness = metalRough.x;
    float finalRoughness = clamp(metalRough.y, 0.04, 1.0);

    // 5. Evaluate Emissive
    vec3 finalEmissive = EvaluateEmissive(ctx.textureMetadataBufferAddr, mat, finalUV);

    // 6. Evaluate Ambient Occlusion
    float finalAo = EvaluateAO(ctx.textureMetadataBufferAddr, mat, finalUV);

    float clearcoatFactor, clearcoatRoughness;
    vec3 clearcoatNormal;
    EvaluateClearcoat(ctx.textureMetadataBufferAddr, mat, finalUV, inNormal, inTangent, frontFacing, clearcoatFactor, clearcoatRoughness, clearcoatNormal);

    float specularFactor;
    vec3 specularColor;
    EvaluateSpecular(ctx.textureMetadataBufferAddr, mat, finalUV, specularFactor, specularColor);

    float ior = mat.ior;

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

    //7. Simulate Lighting
    vec3 totalRadiance = vec3(0.0);

    for(uint i = 0; i < ctx.activeDirectionLightCount && ctx.enableForwardPlusDirectionalLights == 1; ++i) {
        uint entityId = GET_DIRECTION_VISIBLE_LIGHT(ctx.directionLightVisibleIndexBufferAddr, i); 
        uint lightDenseIndex = GET_SPARSE_INDEX(ctx.directionLightSparseMapBufferAddr, entityId);  

        vec3 dirLightDirection = GET_DIRECTION_LIGHT(ctx.directionLightDataBufferAddr, lightDenseIndex).direction.xyz;
        vec3 lightDir = normalize(-dirLightDirection);

        uint debugCascadeIndex = 0;
        float shadowFactor = CalculateDirectionalLightShadow(
            ctx.directionLightShadowDataBufferAddr,
            ctx.directionLightShadowSparseMapBufferAddr,
            entityId,
            worldPos,
            finalNormal,
            lightDir,
            viewDepth,
            dirLightShadowAtlas,
            debugCascadeIndex
        );

        vec3 lightContribution = SimulateDirectionalLight(
            ctx.directionLightDataBufferAddr, lightDenseIndex,
            albedoAlpha.rgb, finalNormal, viewDir, finalRoughness, finalMetalness,
            ior, specularFactor, specularColor, clearcoatFactor, clearcoatRoughness, clearcoatNormal
        );

        totalRadiance += lightContribution * shadowFactor;
    }

    for (uint i = 0; i < cluster.pointLightCount && ctx.enableForwardPlusPointLights == 1; ++i) {
        uint globalLightIndex = cluster.pointLightOffset + i;
        uint lightEntityIndex = GET_LIGHT_INDEX(ctx.forwardPlusPointLightIndexListBufferAddr, globalLightIndex);
        uint lightDenseIndex = GET_SPARSE_INDEX(ctx.pointLightSparseMapBufferAddr, lightEntityIndex);
        
        vec3 pointLightPosition = GET_POINT_LIGHT(ctx.pointLightDataBufferAddr, lightDenseIndex).position.xyz;

        float shadowFactor = CalculatePointLightShadow(
            ctx.pointLightShadowDataBufferAddr,
            ctx.pointLightShadowSparseMapBufferAddr,
            lightEntityIndex,
            worldPos,
            finalNormal,
            pointLightPosition,
            pointLightShadowAtlas
        );

        vec3 lightContribution = SimulatePointLight(
            ctx.pointLightDataBufferAddr, lightDenseIndex, worldPos, 
            albedoAlpha.rgb, finalNormal, viewDir, finalRoughness, finalMetalness,
            ior, specularFactor, specularColor, clearcoatFactor, clearcoatRoughness, clearcoatNormal
        );
        
        totalRadiance += shadowFactor * lightContribution;
    }

    for (uint i = 0; i < cluster.spotLightCount && ctx.enableForwardPlusSpotLights == 1; ++i) {
        uint globalLightIndex = cluster.spotLightOffset + i;
        uint lightEntityIndex = GET_LIGHT_INDEX(ctx.forwardPlusSpotLightIndexListBufferAddr, globalLightIndex);
        uint lightDenseIndex = GET_SPARSE_INDEX(ctx.spotLightSparseMapBufferAddr, lightEntityIndex);
        
        vec3 spotLightPosition = GET_SPOT_LIGHT(ctx.spotLightDataBufferAddr, lightDenseIndex).position.xyz;
        vec3 lightDir = normalize(spotLightPosition - worldPos);

        float shadowFactor = CalculateSpotLightShadow(
            ctx.spotLightShadowDataBufferAddr,
            ctx.spotLightShadowSparseMapBufferAddr,
            lightEntityIndex,
            worldPos,
            finalNormal,
            lightDir,
            spotLightShadowAtlas
        );

        vec3 lightContribution = SimulateSpotLight(
            ctx.spotLightDataBufferAddr, lightDenseIndex, worldPos,
            albedoAlpha.rgb, finalNormal, viewDir, finalRoughness, finalMetalness,
            ior, specularFactor, specularColor, clearcoatFactor, clearcoatRoughness, clearcoatNormal
        );

        totalRadiance += shadowFactor * lightContribution;    
    }

    if(ctx.enableForwardPlusEmissiveAo == 1)
    {
        //IBL/Ambient
        totalRadiance += SimulateEnvironmentLight(
            ctx.environmentBufferAddr, ctx.activeEnvironmentIndex, ctx.brdfLutTextureIndex,
            albedoAlpha.rgb, finalNormal, viewDir, finalRoughness, finalMetalness,
            ior, specularFactor, specularColor, finalAo, ctx.ambientStrength
        );

        //Bloom Radiance
        totalRadiance += SimulateBloom(finalEmissive, 1.0, ctx.emissiveStrength);
    }

    // 8. Write WBOIT Accumulation
    vec3 premultipliedColor = totalRadiance * albedoAlpha.a;
    float weight = calculateWboitWeight(gl_FragCoord.z, albedoAlpha.a);

    outAccum = vec4(premultipliedColor, albedoAlpha.a) * weight;
    outReveal = albedoAlpha.a;
}