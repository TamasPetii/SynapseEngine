#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../../Includes/Core.glsl"
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

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inTangent;
layout(location = 2) in vec2 inUV;
layout(location = 3) in flat uvec4 inId; // (EntityID, MaterialID, MeshletIndex, LodIndex) 

layout(location = 0) out vec4 outColor;

#include "../../../../Includes/PushConstants/MeshletForwardPC.glsl"

layout(push_constant) uniform PushConstants {
   MeshletForwardPC pc;
};

vec3 SimulateDirectionalLight(uint lightIndex, vec3 albedo, vec3 normal, vec3 viewDir, float roughness, float metallic) {
    DirectionLightComponent light = GET_DIRECTION_LIGHT(pc.directionLightDataAddr, lightIndex);
    vec3 lightDir = normalize(-light.direction);
        
    return ShadePhysicallyBased(
        albedo, normal, viewDir, lightDir, roughness, metallic, 
        light.color, 1.0, light.strength
    );
}

vec3 SimulatePointLight(uint lightIndex, vec3 worldPos, vec3 albedo, vec3 normal, vec3 viewDir, float roughness, float metallic) {
    PointLightComponent light = GET_POINT_LIGHT(pc.pointLightDataAddr, lightIndex);
    
    float distToLight = distance(worldPos, light.position);
    if (distToLight > light.radius) return vec3(0.0);

    vec3 lightDir = normalize(light.position - worldPos);
    
    float attenuation = clamp(1.0 - (distToLight * distToLight) / (light.radius * light.radius), 0.0, 1.0);
    attenuation *= attenuation;

    return ShadePhysicallyBased(
        albedo, normal, viewDir, lightDir, roughness, metallic, 
        light.color, attenuation, light.strength
    );
}

vec3 SimulateSpotLight(uint lightIndex, vec3 worldPos, vec3 albedo, vec3 normal, vec3 viewDir, float roughness, float metallic) {
    SpotLightComponent light = GET_SPOT_LIGHT(pc.spotLightDataAddr, lightIndex);
    
    float distToLight = distance(worldPos, light.position);
    if (distToLight > light.range) return vec3(0.0);

    vec3 lightDirToFrag = normalize(worldPos - light.position);
    vec3 spotDirection = normalize(light.direction);
    float theta = dot(lightDirToFrag, spotDirection);
    
    if (theta < light.outerCosAngle) return vec3(0.0);

    float epsilon = light.innerCosAngle - light.outerCosAngle;
    float spotIntensity = clamp((theta - light.outerCosAngle) / epsilon, 0.0, 1.0);

    vec3 lightDir = normalize(light.position - worldPos);
    
    float attenuation = clamp(1.0 - (distToLight * distToLight) / (light.range * light.range), 0.0, 1.0);
    attenuation *= attenuation;
    attenuation *= spotIntensity;

    return ShadePhysicallyBased(
        albedo, normal, viewDir, lightDir, roughness, metallic, 
        light.color, attenuation, light.strength
    );
}

void main() {
    uint entityId = inId.x;
    uint materialId = inId.y;

    Material mat = GET_MATERIAL(pc.materialBuffer, materialId);
    vec2 finalUV = inUV * mat.uvScale;

    vec4 albedoAlpha = mat.color;
    if (HAS_ALBEDO_TEX(mat)) {
        albedoAlpha *= SampleTexture2D(mat.albedoTexture, SAMPLER_LINEAR_ANISO, finalUV);
    }
    
    if (albedoAlpha.a < 0.05) 
        discard;

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

    float finalMetalness = mat.metalness;
    float finalRoughness = mat.roughness;

    if (HAS_METALNESS_TEX(mat)) 
        finalMetalness *= SampleTexture2D(mat.metalnessTexture, SAMPLER_LINEAR_ANISO, finalUV).r;

    if (HAS_ROUGHNESS_TEX(mat)) 
        finalRoughness *= SampleTexture2D(mat.roughnessTexture, SAMPLER_LINEAR_ANISO, finalUV).r;

    if (HAS_METALLIC_ROUGHNESS_TEX(mat)) {
        vec4 mrSample = SampleTexture2D(mat.metallicRoughnessTexture, SAMPLER_LINEAR_ANISO, finalUV);
        finalRoughness *= mrSample.g;
        finalMetalness *= mrSample.b;
    }

    finalRoughness = clamp(finalRoughness, 0.04, 1.0);

    vec3 finalEmissive = mat.emissiveColor * mat.emissiveIntensity;
    if (HAS_EMISSIVE_TEX(mat)) {
        finalEmissive *= SampleTexture2D(mat.emissiveTexture, SAMPLER_LINEAR_ANISO, finalUV).rgb;
    }

    float finalAo = mat.aoStrength;
    if (HAS_AO_TEX(mat)) {
        finalAo *= SampleTexture2D(mat.ambientOcclusionTexture, SAMPLER_LINEAR_ANISO, finalUV).r;
    }

    uint cameraDenseIndex = GET_SPARSE_INDEX(pc.cameraSparseMapBufferAddr, pc.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(pc.cameraBufferAddr, cameraDenseIndex);

    vec2 screenUV = gl_FragCoord.xy / vec2(pc.screenWidth, pc.screenHeight);
    float fragDepth = gl_FragCoord.z;
    vec3 worldPos = ReconstructWorldPosition(screenUV, fragDepth, camera.viewProjVulkanInv);

    vec4 viewPos = camera.view * vec4(worldPos, 1.0);
    float viewDepth = abs(viewPos.z);
    vec3 viewDir = normalize(camera.eye.xyz - worldPos);

    uint tileX = uint(gl_FragCoord.x) / pc.tileSize;
    uint tileY = uint(gl_FragCoord.y) / pc.tileSize;
    uint tileIndex = tileY * pc.tileCountX + tileX;

    TileData tile = GET_TILE_DATA(pc.tileGridListAddr, tileIndex);
    uint sliceIdx = GetClusterSliceIndex(viewDepth, tile.minZ, tile.maxZ, tile.sliceCount);
    
    uint clusterIndex = tile.clusterBaseOffset + sliceIdx;
    ClusterData cluster = GET_CLUSTER_DATA(pc.clusterListAddr, clusterIndex);

    vec3 totalRadiance = vec3(0.0);

    for(uint i = 0; i < pc.dirLightCount; ++i) {
        uint lightDenseIndex = GET_VISIBLE_DIRECTION_LIGHT(pc.visibleDirectionLightAddr, i); 
        totalRadiance += SimulateDirectionalLight(lightDenseIndex, albedoAlpha.rgb, finalNormal, viewDir, finalRoughness, finalMetalness);
    }

    for (uint i = 0; i < cluster.pointLightCount; ++i) {
        uint globalLightIndex = cluster.pointLightOffset + i;
        uint lightEntityIndex = GET_LIGHT_INDEX(pc.pointLightIndexListAddr, globalLightIndex);
        uint lightDenseIndex = GET_SPARSE_INDEX(pc.pointLightSparseMapAddr, lightEntityIndex);
        
        totalRadiance += SimulatePointLight(lightDenseIndex, inWorldPos, albedoAlpha.rgb, finalNormal, viewDir, finalRoughness, finalMetalness);
    }

    for (uint i = 0; i < cluster.spotLightCount; ++i) {
        uint globalLightIndex = cluster.spotLightOffset + i;
        uint lightEntityIndex = GET_LIGHT_INDEX(pc.spotLightIndexListAddr, globalLightIndex);
        uint lightDenseIndex = GET_SPARSE_INDEX(pc.spotLightSparseMapAddr, lightEntityIndex);
        
        totalRadiance += SimulateSpotLight(lightDenseIndex, inWorldPos, albedoAlpha.rgb, finalNormal, viewDir, finalRoughness, finalMetalness);
    }

    vec3 ambient = albedoAlpha.rgb * pc.ambientStrength * finalAo;
    vec3 finalColor = ambient + finalEmissive + totalRadiance;

    outColor = vec4(finalColor, albedoAlpha.a);
}