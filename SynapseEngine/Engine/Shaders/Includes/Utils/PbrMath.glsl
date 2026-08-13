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

#ifndef SYN_INCLUDES_UTILS_PBR_MATH_GLSL
#define SYN_INCLUDES_UTILS_PBR_MATH_GLSL

#include "../Core.glsl"

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float FresnelSchlickScalar(float cosTheta, float F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom + 0.00001);
}

float VisibilitySmithGGXCorrelated(float NdotV, float NdotL, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;

    float lambdaV = NdotL * sqrt(NdotV * NdotV * (1.0 - a2) + a2);
    float lambdaL = NdotV * sqrt(NdotL * NdotL * (1.0 - a2) + a2);

    return 0.5 / max(lambdaV + lambdaL, 0.00001);
}

float VisibilityKelemen(float VdotH) {
    return 0.25 / max(VdotH * VdotH, 0.00001);
}

vec3 ShadePhysicallyBased(
    vec3 albedo, 
    vec3 N, 
    vec3 V, 
    vec3 L, 
    float roughness, 
    float metalness, 
    float ior,
    float specularFactor,
    vec3 specularColor,
    float clearcoatFactor,
    float clearcoatRoughness,
    vec3 clearcoatNormal,
    vec3 lightColor, 
    float attenuation, 
    float strength
) {
    vec3 H_unnormalized = V + L;
    vec3 H = dot(H_unnormalized, H_unnormalized) < 1e-5 ? N : normalize(H_unnormalized);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0001); 
    float VdotH = max(dot(V, H), 0.0);

    if (NdotL <= 0.0) return vec3(0.0);

    float clampedIor = max(ior, 1.0);
    float iorRatio = (clampedIor - 1.0) / (clampedIor + 1.0);
    float f0Dielectric = iorRatio * iorRatio;
    
    vec3 f0DielectricModified = min(f0Dielectric * specularColor * specularFactor, vec3(1.0));
    vec3 F0 = mix(f0DielectricModified, albedo, metalness);

    float alpha = roughness * roughness;
    float D = DistributionGGX(N, H, alpha);
    float Vis = VisibilitySmithGGXCorrelated(NdotV, NdotL, alpha);
    vec3 F = FresnelSchlick(VdotH, F0);

    vec3 baseSpecular = D * Vis * F;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metalness);
    vec3 baseDiffuse = kD * albedo / PI;

    vec3 baseLayer = baseDiffuse + baseSpecular;

    vec3 finalColor = baseLayer;
    
    if (clearcoatFactor > 0.0) {
        vec3 Nc = clearcoatNormal;
        float NcDotL = max(dot(Nc, L), 0.0);
        float NcDotH = max(dot(Nc, H), 0.0);
        
        float ccRoughnessSafe = clamp(clearcoatRoughness, 0.04, 1.0); 
        float ccAlpha = ccRoughnessSafe * ccRoughnessSafe;
        
        float Dc = DistributionGGX(Nc, H, ccAlpha);
        float Vc = VisibilityKelemen(VdotH);
        float Fc = FresnelSchlickScalar(VdotH, 0.04) * clearcoatFactor;

        float clearcoatSpecular = Dc * Vc * Fc;

        finalColor = baseLayer * (1.0 - Fc) + vec3(clearcoatSpecular);
    }

    vec3 radiance = lightColor * attenuation * strength;
    return finalColor * radiance * NdotL;
}

#endif