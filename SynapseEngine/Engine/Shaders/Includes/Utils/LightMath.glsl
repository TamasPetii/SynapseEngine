#ifndef SYN_INCLUDES_UTILS_LIGHT_MATH_GLSL
#define SYN_INCLUDES_UTILS_LIGHT_MATH_GLSL

#include "PbrMath.glsl"
#include "../Common/DirectionLight.glsl"
#include "../Common/PointLight.glsl"
#include "../Common/SpotLight.glsl"

vec3 SimulateAmbientLight(vec3 albedo, float ambientIntensity, float globalAmbientIntensity) {
    return albedo * ambientIntensity * globalAmbientIntensity;
}

vec3 SimulateBloom(vec3 emissiveColor, float emissiveIntensity, float globalEmissiveIntensity) {
    return emissiveColor * emissiveIntensity * globalEmissiveIntensity;
}

vec3 SimulateDirectionalLight(const uint64_t directionLightDataBufferAddr, uint lightIndex, vec3 albedo, vec3 normal, vec3 viewDir, float roughness, float metallic) {
    DirectionLightComponent light = GET_DIRECTION_LIGHT(directionLightDataBufferAddr, lightIndex);
    vec3 lightDir = normalize(-light.direction);
        
    return ShadePhysicallyBased(
        albedo, normal, viewDir, lightDir, roughness, metallic, 
        light.color, 1.0, light.strength
    );
}

vec3 SimulatePointLight(const uint64_t pointLightDataBufferAddr, uint lightIndex, vec3 worldPos, vec3 albedo, vec3 normal, vec3 viewDir, float roughness, float metallic) {
    PointLightComponent light = GET_POINT_LIGHT(pointLightDataBufferAddr, lightIndex);
    
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

vec3 SimulateSpotLight(const uint64_t spotLightDataBufferAddr, uint lightIndex, vec3 worldPos, vec3 albedo, vec3 normal, vec3 viewDir, float roughness, float metallic) {
    SpotLightComponent light = GET_SPOT_LIGHT(spotLightDataBufferAddr, lightIndex);
    
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

#endif