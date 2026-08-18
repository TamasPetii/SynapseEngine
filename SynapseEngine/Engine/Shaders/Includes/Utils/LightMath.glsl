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

#ifndef SYN_INCLUDES_UTILS_LIGHT_MATH_GLSL
#define SYN_INCLUDES_UTILS_LIGHT_MATH_GLSL

#include "PbrMath.glsl"
#include "../Common/DirectionLight.glsl"
#include "../Common/PointLight.glsl"
#include "../Common/SpotLight.glsl"
#include "../Common/Environment.glsl"

vec3 SimulateAmbientLight(vec3 albedo, float ambientIntensity, float globalAmbientIntensity) {
    return albedo * ambientIntensity * globalAmbientIntensity;
}

vec3 SimulateEnvironmentLight(
    uint64_t envBufferAddr, uint envIndex, uint brdfLutIndex,
    vec3 albedo, vec3 normal, vec3 viewDir, float roughness, float metalness,
    float ior, float specularFactor, vec3 specularColor,
    float ao, float globalAmbientStrength
) 
{
    if (envIndex == 0xFFFFFFFF) {
        return SimulateAmbientLight(albedo, ao, globalAmbientStrength);
    }

    EnvironmentData env = GET_ENVIRONMENT(envBufferAddr, envIndex);
    uint samplerIndex = GET_ENV_CUBE_SAMPLER(env);

    vec3 R = reflect(-viewDir, normal);
    float NdotV = max(dot(normal, viewDir), 0.0);

    vec3 irradiance = SampleEnvironmentIrradianceCube(envIndex, samplerIndex, normal).rgb;
    
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefiltered = SampleEnvironmentPrefilteredCube(envIndex, samplerIndex, R, roughness * MAX_REFLECTION_LOD).rgb;
    
    vec2 brdf = SampleTexture2D(brdfLutIndex, samplerIndex, vec2(NdotV, roughness)).rg;

    vec3 iblRadiance = SimulateIBL(
        albedo, normal, viewDir, roughness, metalness,
        ior, specularFactor, specularColor,
        irradiance, prefiltered, brdf
    );

    return iblRadiance * ao * env.skyTint * env.ambientIntensity * globalAmbientStrength;
}

vec3 SimulateBloom(vec3 emissiveColor, float emissiveIntensity, float globalEmissiveIntensity) {
    return emissiveColor * emissiveIntensity * globalEmissiveIntensity;
}

vec3 SimulateDirectionalLight(
    const uint64_t directionLightDataBufferAddr, uint lightIndex, 
    vec3 albedo, vec3 normal, vec3 viewDir, float roughness, float metallic,
    float ior, float specularFactor, vec3 specularColor,
    float clearcoatFactor, float clearcoatRoughness, vec3 clearcoatNormal
) {
    DirectionLightComponent light = GET_DIRECTION_LIGHT(directionLightDataBufferAddr, lightIndex);
    vec3 lightDir = normalize(-light.direction);
        
    return ShadePhysicallyBased(
        albedo, normal, viewDir, lightDir, roughness, metallic, 
        ior, specularFactor, specularColor, clearcoatFactor, clearcoatRoughness, clearcoatNormal,
        light.color, 1.0, light.strength
    );
}

vec3 SimulatePointLight(
    const uint64_t pointLightDataBufferAddr, uint lightIndex, vec3 worldPos, 
    vec3 albedo, vec3 normal, vec3 viewDir, float roughness, float metallic,
    float ior, float specularFactor, vec3 specularColor,
    float clearcoatFactor, float clearcoatRoughness, vec3 clearcoatNormal
) {
    PointLightComponent light = GET_POINT_LIGHT(pointLightDataBufferAddr, lightIndex);
    
    float distToLight = distance(worldPos, light.position);
    if (distToLight > light.radius) return vec3(0.0);

    vec3 lightDir = normalize(light.position - worldPos);
    
    float attenuation = clamp(1.0 - (distToLight * distToLight) / (light.radius * light.radius), 0.0, 1.0);
    attenuation *= attenuation;

    return ShadePhysicallyBased(
        albedo, normal, viewDir, lightDir, roughness, metallic, 
        ior, specularFactor, specularColor, clearcoatFactor, clearcoatRoughness, clearcoatNormal,
        light.color, attenuation, light.strength
    );
}

vec3 SimulateSpotLight(
    const uint64_t spotLightDataBufferAddr, uint lightIndex, vec3 worldPos, 
    vec3 albedo, vec3 normal, vec3 viewDir, float roughness, float metallic,
    float ior, float specularFactor, vec3 specularColor,
    float clearcoatFactor, float clearcoatRoughness, vec3 clearcoatNormal
) {
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
        ior, specularFactor, specularColor, clearcoatFactor, clearcoatRoughness, clearcoatNormal,
        light.color, attenuation, light.strength
    );
}

#endif