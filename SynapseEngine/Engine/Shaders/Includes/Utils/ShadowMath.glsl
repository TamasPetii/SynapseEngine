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

#ifndef SYN_INCLUDES_UTILS_SHADOW_MATH_GLSL
#define SYN_INCLUDES_UTILS_SHADOW_MATH_GLSL

#include "../Common/DirectionLight.glsl"
#include "../Common/PointLight.glsl"
#include "../Common/SpotLight.glsl"

vec3 SampleShadowAtlasPCF(
    sampler2DShadow shadowAtlas,
    sampler2D shadowColorAtlas,
    vec2 uv, vec2 minUV, vec2 maxUV, 
    float currentDepth, vec2 texelSize,
    uint usePCF
) {
    // Opaque Shadow (Hardware PCF or Hard Shadow)
    float opaqueShadow = 0.0;
    
    if (usePCF == 1) 
    {
        for (int x = 0; x <= 1; ++x) {
            for (int y = 0; y <= 1; ++y) {
                vec2 offset = (vec2(x, y) - 0.5) * texelSize;
                vec2 sampleUV = clamp(uv + offset, minUV, maxUV);
                opaqueShadow += texture(shadowAtlas, vec3(sampleUV, currentDepth));
            }
        }
        opaqueShadow /= 4.0;
    } 
    else 
    {
        vec2 sampleUV = clamp(uv, minUV, maxUV);
        opaqueShadow = texture(shadowAtlas, vec3(sampleUV, currentDepth));
    }

    // Early out: if fully occluded by opaque geometry
    if (opaqueShadow == 0.0) 
        return vec3(0.0);

    // Transparent Shadow
    vec3 transparentShadow = vec3(0.0);
    
    if (usePCF == 1) {
        // Optimized 2x2 PCF using Nearest sampler
        for (int x = 0; x <= 1; ++x) {
            for (int y = 0; y <= 1; ++y) {
                vec2 offset = (vec2(x, y) - 0.5) * texelSize;
                vec2 sampleUV = clamp(uv + offset, minUV, maxUV);
            
                vec4 transData = textureLod(shadowColorAtlas, sampleUV, 0.0);
            
                if (currentDepth > transData.a) {
                    transparentShadow += transData.rgb;
                } else {
                    transparentShadow += vec3(1.0);
                }
            }
        }
        transparentShadow /= 4.0;
    } 
    else 
    {
        // Hard Transparent Shadow (Single fetch)
        vec2 sampleUV = clamp(uv, minUV, maxUV);
        vec4 transData = textureLod(shadowColorAtlas, sampleUV, 0.0);
        
        if (currentDepth > transData.a) {
            transparentShadow = transData.rgb;
        } else {
            transparentShadow = vec3(1.0);
        }
    }
    
    return vec3(opaqueShadow) * transparentShadow;
}

vec3 CalculateDirectionalLightShadow(
    const uint64_t dirLightShadowDataBufferAddr,
    const uint64_t dirLightShadowSparseMapBufferAddr,
    uint lightEntityIndex,
    vec3 worldPos,
    vec3 normal,
    vec3 lightDir,
    float viewDepth,
    sampler2DShadow dynamicShadowAtlas,
    sampler2D dynamicShadowColorAtlas,
    sampler2DShadow staticShadowAtlas,
    sampler2D staticShadowColorAtlas,
    out uint outCascadeIndex
) {
    outCascadeIndex = 0;

    if (dirLightShadowSparseMapBufferAddr == 0) return vec3(1.0);
    
    uint shadowDenseIndex = GET_SPARSE_INDEX(dirLightShadowSparseMapBufferAddr, lightEntityIndex);
    if (shadowDenseIndex == INVALID_INDEX) return vec3(1.0);

    DirectionLightShadowComponent shadowComp = GET_DIRECTION_LIGHT_SHADOW(dirLightShadowDataBufferAddr, shadowDenseIndex);

    // Select cascade based on absolute view-space depth
    uint cascadeIndex = 0;
    for (uint i = 0; i < 3; ++i) {
        if (viewDepth > shadowComp.cascadeSplits[i]) {
            cascadeIndex = i + 1;
        }
    }
    outCascadeIndex = cascadeIndex;

    vec4 clipPos = shadowComp.cascadeViewProjsVulkan[cascadeIndex] * vec4(worldPos, 1.0);
    vec3 ndc = clipPos.xyz / (clipPos.w == 0.0 ? 1.0 : clipPos.w);
    
    if (ndc.z < 0.0 || ndc.z > 1.0 || ndc.x < -1.0 || ndc.x > 1.0 || ndc.y < -1.0 || ndc.y > 1.0) {
        return vec3(1.0); 
    }

    float NoL = clamp(dot(normal, lightDir), 0.0, 1.0);
    float tanTheta = sqrt(1.0 - NoL * NoL) / (NoL + 0.0001);
    float cascadeMultiplier = 1.0 + (float(cascadeIndex) * 0.5);
    float baseConstantBias = 0.0001; 
    float baseSlopeBias = 0.0005;
    float totalBias = (baseConstantBias + baseSlopeBias * tanTheta) * cascadeMultiplier;
    totalBias = min(totalBias, 0.0025); 
    float currentDepth = ndc.z - totalBias;

    // Map NDC to shadow atlas UV coordinates
    vec2 uv = ndc.xy * 0.5 + 0.5;
    vec4 rect = shadowComp.cascadeAtlasRects[cascadeIndex];
    uv = uv * rect.zw + rect.xy;

    // Clamp UV with half-texel margin to prevent cascade bleeding
    vec2 texelSize = 1.0 / vec2(textureSize(dynamicShadowAtlas, 0));
    vec2 minUV = rect.xy + (texelSize * 0.5); 
    vec2 maxUV = rect.xy + rect.zw - (texelSize * 0.5);
    uv = clamp(uv, minUV, maxUV);

    vec3 dynamicShadow = SampleShadowAtlasPCF(dynamicShadowAtlas, dynamicShadowColorAtlas, uv, minUV, maxUV, currentDepth, texelSize, 1);

    if (dynamicShadow == vec3(0.0))
        return vec3(0.0);

    vec3 staticShadow = SampleShadowAtlasPCF(staticShadowAtlas, staticShadowColorAtlas, uv, minUV, maxUV, currentDepth, texelSize, 1);

    return dynamicShadow * staticShadow;
}

vec3 CalculateSpotLightShadow(
    const uint64_t spotLightShadowDataBufferAddr,
    const uint64_t spotLightShadowSparseMapBufferAddr,
    uint lightEntityIndex,
    vec3 worldPos,
    vec3 normal,
    vec3 lightDir,
    sampler2DShadow shadowAtlas,
    sampler2D shadowColorAtlas
) {
    if (spotLightShadowSparseMapBufferAddr == 0) return vec3(1.0);

    uint shadowDenseIndex = GET_SPARSE_INDEX(spotLightShadowSparseMapBufferAddr, lightEntityIndex);
    if (shadowDenseIndex == INVALID_INDEX) return vec3(1.0);

    SpotLightShadowComponent shadowComp = GET_SPOT_LIGHT_SHADOW(spotLightShadowDataBufferAddr, shadowDenseIndex);

    // 1. Project to clip space
    vec4 clipPos = shadowComp.viewProj * vec4(worldPos, 1.0);
    if (clipPos.w <= 0.0) return vec3(1.0);

    vec3 ndc = clipPos.xyz / clipPos.w;
    if (ndc.z < 0.0 || ndc.z > 1.0 || ndc.x < -1.0 || ndc.x > 1.0 || ndc.y < -1.0 || ndc.y > 1.0) {
        return vec3(1.0);
    }

    float near = shadowComp.planes.x;
    float far = shadowComp.planes.y;
    float linearDist = clipPos.w;

    float NoL = clamp(dot(normal, lightDir), 0.0, 1.0);
    float tanTheta = sqrt(1.0 - NoL * NoL) / (NoL + 0.0001);
    
    float constantBias = 0.1; 
    float slopeBias = 0.2;
    float linearBias = constantBias + slopeBias * tanTheta;
    float biasedDist = max(linearDist - linearBias, near + 0.001);

    // Convert biased linear distance back to non-linear Vulkan depth [0, 1]
    float currentDepth = (far / (far - near)) - (far * near) / ((far - near) * biasedDist);

    // 3. NDC to Atlas UV mapping
    vec2 uv = ndc.xy * 0.5 + 0.5;
    vec4 rect = shadowComp.atlasRect;
    uv = uv * rect.zw + rect.xy;

    // Half-texel clamp to prevent atlas bleeding
    vec2 texelSize = 1.0 / vec2(textureSize(shadowAtlas, 0));
    vec2 minUV = rect.xy + (texelSize * 0.5); 
    vec2 maxUV = rect.xy + rect.zw - (texelSize * 0.5);
    uv = clamp(uv, minUV, maxUV);

    return SampleShadowAtlasPCF(shadowAtlas, shadowColorAtlas, uv, minUV, maxUV, currentDepth, texelSize, 1);
}

vec3 CalculatePointLightShadow(
    const uint64_t pointLightShadowDataBufferAddr,
    const uint64_t pointLightShadowSparseMapBufferAddr,
    uint lightEntityIndex,
    vec3 worldPos,
    vec3 normal,
    vec3 lightPos,
    sampler2DShadow shadowAtlas,
    sampler2D shadowColorAtlas
) {
    if (pointLightShadowSparseMapBufferAddr == 0) return vec3(1.0);

    uint shadowDenseIndex = GET_SPARSE_INDEX(pointLightShadowSparseMapBufferAddr, lightEntityIndex);
    if (shadowDenseIndex == INVALID_INDEX) return vec3(1.0);

    PointLightShadowComponent shadowComp = GET_POINT_LIGHT_SHADOW(pointLightShadowDataBufferAddr, shadowDenseIndex);

    vec3 lightToFrag = worldPos - lightPos;
    vec3 absVec = abs(lightToFrag);

    // 1. Select face based on dominant axis
    uint faceIndex = 0;
    if (absVec.x >= absVec.y && absVec.x >= absVec.z) {
        faceIndex = (lightToFrag.x > 0.0) ? 0 : 1;
    } else if (absVec.y >= absVec.x && absVec.y >= absVec.z) {
        faceIndex = (lightToFrag.y > 0.0) ? 2 : 3;
    } else {
        faceIndex = (lightToFrag.z > 0.0) ? 4 : 5;
    }

    // 2. Project to get NDC xy
    vec4 clipPos = shadowComp.viewProjs[faceIndex] * vec4(worldPos, 1.0);
    if (clipPos.w <= 0.0) return vec3(1.0);

    vec3 ndc = clipPos.xyz / clipPos.w;
    if (ndc.z < 0.0 || ndc.z > 1.0 || ndc.x < -1.0 || ndc.x > 1.0 || ndc.y < -1.0 || ndc.y > 1.0) {
        return vec3(1.0);
    }

    // 3. Linear Slope-Scaled Bias Calculation
    float near = shadowComp.planes.x;
    float far = shadowComp.planes.y;
    
    // Linear distance to light along the dominant axis
    float linearDist = max(absVec.x, max(absVec.y, absVec.z));

    vec3 lightDir = normalize(lightToFrag);
    float NoL = clamp(dot(normal, -lightDir), 0.0, 1.0);
    float tanTheta = sqrt(1.0 - NoL * NoL) / (NoL + 0.0001);
    
    // Bias values in world-space units
    float constantBias = 0.1; 
    float slopeBias = 0.2;
    float linearBias = constantBias + slopeBias * tanTheta;
    float biasedDist = max(linearDist - linearBias, near + 0.001);

    // Convert biased linear distance back to non-linear Vulkan depth [0, 1]
    float currentDepth = (far / (far - near)) - (far * near) / ((far - near) * biasedDist);

    // 4. NDC to Atlas UV mapping
    vec2 uv = ndc.xy * 0.5 + 0.5;
    vec4 rect = shadowComp.atlasRects[faceIndex];
    uv = uv * rect.zw + rect.xy;

    // Half-texel clamp to prevent atlas bleeding
    vec2 texelSize = 1.0 / vec2(textureSize(shadowAtlas, 0));
    vec2 minUV = rect.xy + (texelSize * 0.5); 
    vec2 maxUV = rect.xy + rect.zw - (texelSize * 0.5);
    uv = clamp(uv, minUV, maxUV);

    return SampleShadowAtlasPCF(shadowAtlas, shadowColorAtlas, uv, minUV, maxUV, currentDepth, texelSize, 1);
}

#endif