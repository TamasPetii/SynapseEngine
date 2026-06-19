#ifndef SYN_INCLUDES_UTILS_OCCLUSION_GLSL
#define SYN_INCLUDES_UTILS_OCCLUSION_GLSL

#include "../Common/Camera.glsl"

// Zeux Approximate Sphere Projection (Változatlan)
bool ProjectSphere(vec3 viewCenter, float radius, mat4 proj, float near, out vec4 uvBounds) {
    if (-viewCenter.z - radius < near) return false;
    vec2 cx = vec2(viewCenter.x, -viewCenter.z);
    vec2 vx = vec2(sqrt(dot(cx, cx) - radius * radius), radius);
    vec2 minx = mat2(vx.x, vx.y, -vx.y, vx.x) * cx;
    vec2 maxx = mat2(vx.x, -vx.y, vx.y, vx.x) * cx;
    vec2 cy = vec2(viewCenter.y, -viewCenter.z);
    vec2 vy = vec2(sqrt(dot(cy, cy) - radius * radius), radius);
    vec2 miny = mat2(vy.x, vy.y, -vy.y, vy.x) * cy;
    vec2 maxy = mat2(vy.x, -vy.y, vy.y, vy.x) * cy;
    float p00 = abs(proj[0][0]);
    float p11 = abs(proj[1][1]);

    uvBounds = vec4(minx.x / minx.y * p00, miny.x / miny.y * p11, 
                    maxx.x / maxx.y * p00, maxy.x / maxy.y * p11);
    uvBounds = uvBounds * 0.5 + 0.5;

    return true;
}

float CalculateSphereScreenSize(vec3 worldCenter, float radius, mat4 view, mat4 proj, float near, vec2 screenRes) {
    vec3 viewCenter = (view * vec4(worldCenter, 1.0)).xyz;
    vec4 uvBounds;
    
    if (ProjectSphere(viewCenter, radius, proj, near, uvBounds)) {
        vec2 sizeInPixels = (vec2(uvBounds.zw) - vec2(uvBounds.xy)) * screenRes;
        return max(sizeInPixels.x, sizeInPixels.y);
    }
    
    return 99999.0;
}

bool ProjectSphereOrtho(vec3 worldCenter, float worldRadius, mat4 viewProj, out vec4 cascadeUVBounds, out float closestZ) {
    vec4 clipCenter = viewProj * vec4(worldCenter, 1.0);
    
    // Scale radius using orthographic projection matrix extents
    float radiusNDC_X = worldRadius * abs(viewProj[0][0]);
    float radiusNDC_Y = worldRadius * abs(viewProj[1][1]);
    float maxRadiusNDC = max(radiusNDC_X, radiusNDC_Y);

    vec2 ndcMin = clipCenter.xy - maxRadiusNDC;
    vec2 ndcMax = clipCenter.xy + maxRadiusNDC;

    // Map NDC [-1, 1] to UV [0, 1]
    cascadeUVBounds = vec4(ndcMin * 0.5 + 0.5, ndcMax * 0.5 + 0.5);

    // Calculate closest Z (Vulkan: 0.0 near, 1.0 far)
    float radiusNDC_Z = worldRadius * abs(viewProj[2][2]);
    closestZ = clipCenter.z - radiusNDC_Z;

    return true;
}

bool IsSphereOccluded(vec3 worldCenter, float radius, CameraComponent camera, sampler2D depthPyramid, vec2 screenRes, bool enableDepthOcclusion, out float outScreenSizePixels) {
    vec3 viewCenter = (camera.view * vec4(worldCenter, 1.0)).xyz;
    vec4 uv;
    
    outScreenSizePixels = 99999.0;

    if (ProjectSphere(viewCenter, radius, camera.proj, camera.params.x, uv)) {
        vec2 sizeInPixels = (vec2(uv.zw) - vec2(uv.xy)) * screenRes;
        outScreenSizePixels = max(sizeInPixels.x, sizeInPixels.y);

        if (outScreenSizePixels < 1.0) {
            return true; 
        }

        if (enableDepthOcclusion) {
            float lod = max(0.0, ceil(log2(outScreenSizePixels * 0.5)));

            /* DO NOT USE MAX REDUCTION SAMPLER AND UV COORDS!!!
            */
            vec2 centerUV = (uv.xy + uv.zw) * 0.5;
            centerUV.y = 1.0 - centerUV.y;            
            float maxDepth = textureLod(depthPyramid, centerUV, lod).r;

            /*
            int mipLevel = int(lod);
            ivec2 mipSize = textureSize(depthPyramid, mipLevel);

            vec2 uvMin = vec2(uv.x, 1.0 - uv.w);
            vec2 uvMax = vec2(uv.z, 1.0 - uv.y);

            vec2 texCoordMin = uvMin * vec2(mipSize);
            vec2 texCoordMax = uvMax * vec2(mipSize);

            ivec2 p0 = clamp(ivec2(texCoordMin), ivec2(0), mipSize - 1);
            ivec2 p1 = clamp(ivec2(texCoordMax), ivec2(0), mipSize - 1);

            float d00 = texelFetch(depthPyramid, p0, mipLevel).r;
            float d10 = texelFetch(depthPyramid, ivec2(p1.x, p0.y), mipLevel).r;
            float d01 = texelFetch(depthPyramid, ivec2(p0.x, p1.y), mipLevel).r;
            float d11 = texelFetch(depthPyramid, p1, mipLevel).r;

            float maxDepth = max(max(d00, d10), max(d01, d11));
            */

            float sphereClosestDepth = -viewCenter.z - radius;
            float normalizedDepth = (sphereClosestDepth - camera.params.x) / (camera.params.y - camera.params.x);

            return normalizedDepth > maxDepth;
        }
    }

    return false;
}

bool IsSphereOccludedDirLightShadow(vec3 worldCenter, float radius, mat4 viewProj, vec4 atlasRect, sampler2D shadowDepthPyramid, float atlasSize, uint maxHizMipLevel, out float outScreenSizePixels) {
    vec4 cascadeUVBounds;
    float closestZ;
    
    outScreenSizePixels = 99999.0;

    if (ProjectSphereOrtho(worldCenter, radius, viewProj, cascadeUVBounds, closestZ)) 
    {
        // Map local cascade UVs to global atlas UVs
        vec2 atlasUV_min = atlasRect.xy + cascadeUVBounds.xy * atlasRect.zw;
        vec2 atlasUV_max = atlasRect.xy + cascadeUVBounds.zw * atlasRect.zw;

        // Strict clamp to prevent bleeding into adjacent cascades during HZB sampling
        vec2 atlasLimitMin = atlasRect.xy;
        vec2 atlasLimitMax = atlasRect.xy + atlasRect.zw;
        atlasUV_min = clamp(atlasUV_min, atlasLimitMin, atlasLimitMax);
        atlasUV_max = clamp(atlasUV_max, atlasLimitMin, atlasLimitMax);

        vec2 sizeInPixels = (atlasUV_max - atlasUV_min) * atlasSize;
        outScreenSizePixels = max(sizeInPixels.x, sizeInPixels.y);

        // Sub-pixel culling
        if (outScreenSizePixels < 1.0) {
            return true;
        }

        // Calculate HZB LOD (scaled to fit footprint into a 2x2 texel quad)
        float lod = max(0.0, ceil(log2(outScreenSizePixels * 0.5)));

        /* DO NOT USE MAX REDUCTION SAMPLER AND UV COORDS!!!
        */
        lod = clamp(lod, 0.0, maxHizMipLevel);
        vec2 centerUV = (atlasUV_min + atlasUV_max) * 0.5;
        float maxDepth = textureLod(shadowDepthPyramid, centerUV, lod).r;

        /*
        int mipLevel = int(clamp(lod, 0.0, float(maxHizMipLevel)));
        ivec2 mipSize = textureSize(shadowDepthPyramid, mipLevel);

        vec2 texCoordMin = atlasUV_min * vec2(mipSize);
        vec2 texCoordMax = atlasUV_max * vec2(mipSize);
        ivec2 texLimitMin = ivec2(atlasLimitMin * vec2(mipSize));
        ivec2 texLimitMax = clamp(ivec2(atlasLimitMax * vec2(mipSize)) - ivec2(1), ivec2(0), mipSize - ivec2(1));

        ivec2 p0 = clamp(ivec2(texCoordMin), texLimitMin, texLimitMax);
        ivec2 p1 = clamp(ivec2(texCoordMax), texLimitMin, texLimitMax);

        float d00 = texelFetch(shadowDepthPyramid, p0, mipLevel).r;
        float d10 = texelFetch(shadowDepthPyramid, ivec2(p1.x, p0.y), mipLevel).r;
        float d01 = texelFetch(shadowDepthPyramid, ivec2(p0.x, p1.y), mipLevel).r;
        float d11 = texelFetch(shadowDepthPyramid, p1, mipLevel).r;
        float maxDepth = max(max(d00, d10), max(d01, d11));
        */

        // Occluded if the closest sphere point is behind the maximum recorded depth
        return closestZ > maxDepth;
    }

    return false;
}



#endif