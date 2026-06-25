#ifndef SYN_INCLUDES_UTILS_SHADOW_MATH_GLSL
#define SYN_INCLUDES_UTILS_SHADOW_MATH_GLSL

#include "../Common/DirectionLight.glsl"
#include "../Common/PointLight.glsl"
#include "../Common/SpotLight.glsl"

float CalculateDirectionalLightShadow(
    const uint64_t dirLightShadowDataBufferAddr,
    const uint64_t dirLightShadowSparseMapBufferAddr,
    uint lightEntityIndex,
    vec3 worldPos,
    vec3 normal,
    vec3 lightDir,
    float viewDepth,
    sampler2DShadow shadowAtlas,
    out uint outCascadeIndex
) {
    outCascadeIndex = 0;

    if (dirLightShadowSparseMapBufferAddr == 0) return 1.0;
    
    uint shadowDenseIndex = GET_SPARSE_INDEX(dirLightShadowSparseMapBufferAddr, lightEntityIndex);
    if (shadowDenseIndex == INVALID_INDEX) return 1.0;

    DirectionLightShadowComponent shadowComp = GET_DIRECTION_LIGHT_SHADOW(dirLightShadowDataBufferAddr, shadowDenseIndex);

    // 1. Cascade selection based on view-space depth
    uint cascadeIndex = 0;
    for (uint i = 0; i < 3; ++i) {
        if (viewDepth > shadowComp.cascadeSplits[i]) {
            cascadeIndex = i + 1;
        }
    }

    outCascadeIndex = cascadeIndex;

    float NoL = clamp(dot(normal, lightDir), 0.001, 1.0);
    float offsetScale = clamp(1.0 - NoL, 0.0, 1.0);
    float orthoWidth = 1.0 / abs(shadowComp.cascadeViewProjsVulkan[cascadeIndex][0][0]);
    float normalOffsetAmount = orthoWidth * 0.015; 
    vec3 biasedWorldPos = worldPos + normal * (normalOffsetAmount * offsetScale);
    vec4 clipPos = shadowComp.cascadeViewProjsVulkan[cascadeIndex] * vec4(biasedWorldPos, 1.0);
    vec3 ndc = clipPos.xyz / (clipPos.w == 0.0 ? 1.0 : clipPos.w);
    float depthBias = 0.0005 * float(cascadeIndex + 1);
    float currentDepth = ndc.z - depthBias;

    // 2. Clip Space
    if (ndc.x < -1.0 || ndc.x > 1.0 || ndc.y < -1.0 || ndc.y > 1.0) {
        return 1.0; 
    }

    // 3. NDC to Atlas UV
    vec2 uv = ndc.xy * 0.5 + 0.5;
    vec4 rect = shadowComp.cascadeAtlasRects[cascadeIndex];
    uv = uv * rect.zw + rect.xy;

    // 4. Clamp UV with half-texel margin
    vec2 texelSize = 1.0 / vec2(textureSize(shadowAtlas, 0));
    vec2 minUV = rect.xy + (texelSize * 0.5); 
    vec2 maxUV = rect.xy + rect.zw - (texelSize * 0.5);
    uv = clamp(uv, minUV, maxUV);

    // 5. 3x3 PCF
    float shadow = 0.0;
    
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            vec2 sampleUV = clamp(uv + offset, minUV, maxUV);
            shadow += texture(shadowAtlas, vec3(sampleUV, currentDepth));
        }
    }
    
    return shadow / 9.0;
}

float CalculateSpotLightShadow(
    const uint64_t spotLightShadowDataBufferAddr,
    const uint64_t spotLightShadowSparseMapBufferAddr,
    uint lightEntityIndex,
    vec3 worldPos,
    vec3 normal,
    vec3 lightDir,
    sampler2DShadow shadowAtlas
) {
    if (spotLightShadowSparseMapBufferAddr == 0) return 1.0;

    uint shadowDenseIndex = GET_SPARSE_INDEX(spotLightShadowSparseMapBufferAddr, lightEntityIndex);
    if (shadowDenseIndex == INVALID_INDEX) return 1.0;

    SpotLightShadowComponent shadowComp = GET_SPOT_LIGHT_SHADOW(spotLightShadowDataBufferAddr, shadowDenseIndex);

    // 1. Project to clip space
    vec4 clipPos = shadowComp.viewProj * vec4(worldPos, 1.0);
    if (clipPos.w <= 0.0) return 1.0;

    vec3 ndc = clipPos.xyz / clipPos.w;
    if (ndc.z < 0.0 || ndc.z > 1.0 || ndc.x < -1.0 || ndc.x > 1.0 || ndc.y < -1.0 || ndc.y > 1.0) {
        return 1.0;
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

    // 4. 3x3 PCF filter
    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            vec2 sampleUV = clamp(uv + offset, minUV, maxUV);
            shadow += texture(shadowAtlas, vec3(sampleUV, currentDepth));
        }
    }
    
    return shadow / 9.0;
}

float CalculatePointLightShadow(
    const uint64_t pointLightShadowDataBufferAddr,
    const uint64_t pointLightShadowSparseMapBufferAddr,
    uint lightEntityIndex,
    vec3 worldPos,
    vec3 normal,
    vec3 lightPos,
    sampler2DShadow shadowAtlas
) {
    if (pointLightShadowSparseMapBufferAddr == 0) return 1.0;

    uint shadowDenseIndex = GET_SPARSE_INDEX(pointLightShadowSparseMapBufferAddr, lightEntityIndex);
    if (shadowDenseIndex == INVALID_INDEX) return 1.0;

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
    if (clipPos.w <= 0.0) return 1.0;

    vec3 ndc = clipPos.xyz / clipPos.w;
    if (ndc.z < 0.0 || ndc.z > 1.0 || ndc.x < -1.0 || ndc.x > 1.0 || ndc.y < -1.0 || ndc.y > 1.0) {
        return 1.0;
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
    // Formula: z_ndc = (f / (f - n)) - (f * n) / ((f - n) * z_linear)
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

    // 5. 3x3 PCF filter
    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 offset = vec2(x, y) * texelSize;
            vec2 sampleUV = clamp(uv + offset, minUV, maxUV);
            shadow += texture(shadowAtlas, vec3(sampleUV, currentDepth));
        }
    }
    
    return shadow / 9.0;
}

#endif