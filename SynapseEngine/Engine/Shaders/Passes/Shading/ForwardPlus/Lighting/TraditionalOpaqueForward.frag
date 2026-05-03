#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../../../../Includes/Core.glsl"
#include "../../../../Includes/Common/Camera.glsl"
#include "../../../../Includes/Common/Material.glsl"
#include "../../../../Includes/Common/Texture.glsl"
#include "../../../../Includes/Common/Cluster.glsl"
#include "../../../../Includes/Common/PointLight.glsl"
#include "../../../../Includes/Common/SpotLight.glsl"
#include "../../../../Includes/Common/DirectionLight.glsl"
#include "../../../../Includes/Utils/ColorMath.glsl"
#include "../../../../Includes/Utils/PbrMath.glsl"
#include "../../../../Includes/Utils/DepthMath.glsl"
#include "../../../../Includes/Utils/ClusterMath.glsl"

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 inTangent;
layout(location = 2) in vec2 inUV;
layout(location = 3) in flat uvec4 inId; // (EntityID, MaterialID, MeshIndex, LodIndex)

layout(location = 0) out vec4 outColor;

#include "../../../../Includes/PushConstants/TraditionalForwardPC.glsl"

layout(push_constant) uniform PushConstants {
   TraditionalForwardPC pc;
};

vec3 SimulatePointLight(uint lightIndex, vec3 worldPos, vec3 albedo, vec3 normal, vec3 viewDir, float roughness, float metallic) {
    PointLightComponent light = GET_POINT_LIGHT(pc.pointLightDataAddr, lightIndex);
    float d = distance(worldPos, light.position);
    if (d > light.radius) return vec3(0.0);
    float atten = clamp(1.0 - (d*d)/(light.radius*light.radius), 0.0, 1.0);
    return ShadePhysicallyBased(albedo, normal, viewDir, normalize(light.position - worldPos), roughness, metallic, light.color, atten * atten, light.strength);
}

vec3 SimulateSpotLight(uint lightIndex, vec3 worldPos, vec3 albedo, vec3 normal, vec3 viewDir, float roughness, float metallic) {
    SpotLightComponent light = GET_SPOT_LIGHT(pc.spotLightDataAddr, lightIndex);
    float d = distance(worldPos, light.position);
    if (d > light.range) return vec3(0.0);
    vec3 L = normalize(light.position - worldPos);
    float theta = dot(-L, normalize(light.direction));
    if (theta < light.outerCosAngle) return vec3(0.0);
    float spot = clamp((theta - light.outerCosAngle) / (light.innerCosAngle - light.outerCosAngle), 0.0, 1.0);
    float atten = clamp(1.0 - (d*d)/(light.range*light.range), 0.0, 1.0);
    return ShadePhysicallyBased(albedo, normal, viewDir, L, roughness, metallic, light.color, atten * atten * spot, light.strength);
}

void main() {
    uint materialId = inId.y;
    
    // 1. Kamera és Pozíció rekonstrukció
    uint cameraIdx = GET_SPARSE_INDEX(pc.cameraSparseMapBufferAddr, pc.activeCameraEntity);
    CameraComponent camera = GET_CAMERA(pc.cameraBufferAddr, cameraIdx);
    
    vec2 screenUV = gl_FragCoord.xy / vec2(pc.screenWidth, pc.screenHeight);
    vec3 worldPos = ReconstructWorldPosition(screenUV, gl_FragCoord.z, camera.viewProjVulkanInv);
    vec3 viewDir = normalize(camera.eye.xyz - worldPos);
    float viewDepth = abs((camera.viewVulkan * vec4(worldPos, 1.0)).z);

    // 2. Material
    Material mat = GET_MATERIAL(pc.materialBuffer, materialId);
    vec2 finalUV = inUV * mat.uvScale;
    vec4 albedo = mat.color;
    if (HAS_ALBEDO_TEX(mat)) albedo *= SampleTexture2D(mat.albedoTexture, SAMPLER_LINEAR_ANISO, finalUV);
    if (albedo.a < 0.05) discard;

    vec3 normal = normalize(inNormal);
    if (HAS_NORMAL_TEX(mat)) {
        vec3 T = normalize(inTangent.xyz);
        T = normalize(T - normal * dot(normal, T));
        mat3 TBN = mat3(T, cross(normal, T) * inTangent.w, normal);
        normal = normalize(TBN * (SampleTexture2D(mat.normalTexture, SAMPLER_LINEAR_ANISO, finalUV).rgb * 2.0 - 1.0));
    }

    float metallic = mat.metalness;
    float roughness = mat.roughness;
    if (HAS_METALNESS_TEX(mat)) metallic *= SampleTexture2D(mat.metalnessTexture, SAMPLER_LINEAR_ANISO, finalUV).r;
    if (HAS_ROUGHNESS_TEX(mat)) roughness *= SampleTexture2D(mat.roughnessTexture, SAMPLER_LINEAR_ANISO, finalUV).r;
    roughness = clamp(roughness, 0.04, 1.0);

    // 3. Cluster lookup
    uint tileIdx = uint(gl_FragCoord.y / pc.tileSize) * pc.tileCountX + uint(gl_FragCoord.x / pc.tileSize);
    TileData tile = GET_TILE_DATA(pc.tileGridListAddr, tileIdx);
    uint clusterIdx = tile.clusterBaseOffset + GetClusterSliceIndex(viewDepth, tile.minZ, tile.maxZ, tile.sliceCount);
    ClusterData cluster = GET_CLUSTER_DATA(pc.clusterListAddr, clusterIdx);

    // 4. Lighting Accumulation
    vec3 radiance = vec3(0.0);
    
    // Directional
    for(uint i = 0; i < pc.directionLightCount; ++i) {
        uint lIdx = GET_VISIBLE_DIRECTION_LIGHT(pc.visibleDirectionLightAddr, i);
        DirectionLightComponent l = GET_DIRECTION_LIGHT(pc.directionLightDataAddr, lIdx);
        radiance += ShadePhysicallyBased(albedo.rgb, normal, viewDir, normalize(-l.direction), roughness, metallic, l.color, 1.0, l.strength);
    }

    // Point
    for(uint i = 0; i < cluster.pointLightCount; ++i) {
        uint entityIdx = GET_LIGHT_INDEX(pc.pointLightIndexListAddr, cluster.pointLightOffset + i);
        radiance += SimulatePointLight(GET_SPARSE_INDEX(pc.pointLightSparseMapAddr, entityIdx), worldPos, albedo.rgb, normal, viewDir, roughness, metallic);
    }

    // Spot
    for(uint i = 0; i < cluster.spotLightCount; ++i) {
        uint entityIdx = GET_LIGHT_INDEX(pc.spotLightIndexListAddr, cluster.spotLightOffset + i);
        radiance += SimulateSpotLight(GET_SPARSE_INDEX(pc.spotLightSparseMapAddr, entityIdx), worldPos, albedo.rgb, normal, viewDir, roughness, metallic);
    }

    // Emissive + AO
    float ao = mat.aoStrength;
    if (HAS_AO_TEX(mat)) ao *= SampleTexture2D(mat.ambientOcclusionTexture, SAMPLER_LINEAR_ANISO, finalUV).r;
    vec3 emissive = mat.emissiveColor * mat.emissiveIntensity;
    if (HAS_EMISSIVE_TEX(mat)) emissive *= SampleTexture2D(mat.emissiveTexture, SAMPLER_LINEAR_ANISO, finalUV).rgb;

    outColor = vec4(albedo.rgb * pc.ambientStrength * ao + emissive + radiance, albedo.a);
}