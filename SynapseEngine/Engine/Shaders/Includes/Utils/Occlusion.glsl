#ifndef SYN_INCLUDES_UTILS_OCCLUSION_GLSL
#define SYN_INCLUDES_UTILS_OCCLUSION_GLSL

#include "../Common/Camera.glsl"

// Zeux Approximate Sphere Projection
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

// Hierarchical Z-Buffer Occlusion Test
bool IsSphereOccluded(vec3 worldCenter, float radius, CameraComponent camera, sampler2D depthPyramid, vec2 screenRes) {
    vec3 viewCenter = (camera.view * vec4(worldCenter, 1.0)).xyz;
    vec4 uv; // (minX, minY, maxX, maxY)

    if (ProjectSphere(viewCenter, radius, camera.proj, camera.params.x, uv)) {
        vec2 sizeInPixels = (vec2(uv.zw) - vec2(uv.xy)) * screenRes;

        if (max(sizeInPixels.x, sizeInPixels.y) < 1.0) {
            return true; 
        }

        float lod = max(0.0, ceil(log2(max(sizeInPixels.x, sizeInPixels.y) / 2.0)));

        vec2 centerUV = (uv.xy + uv.zw) * 0.5;
        centerUV.y = 1.0 - centerUV.y;
        
        float maxDepth = textureLod(depthPyramid, centerUV, lod).r;
        
        float sphereClosestDepth = -viewCenter.z - radius;
        float normalizedDepth = (sphereClosestDepth - camera.params.x) / (camera.params.y - camera.params.x);

        return normalizedDepth > maxDepth;
    }

    return false;
}

#endif