#ifndef SYN_INCLUDES_UTILS_CULLING_MATH_GLSL
#define SYN_INCLUDES_UTILS_CULLING_MATH_GLSL

#include "../Common/Camera.glsl"
#include "../Common/Mesh.glsl"

#define INTERSECTION_OUTSIDE   0u
#define INTERSECTION_INTERSECT 1u
#define INTERSECTION_INSIDE    2u

uint TestSphereFrustum(vec3 center, float radius, CameraComponent camera) {
    bool isIntersecting = false;
    for(int i = 0; i < 6; ++i) {
        vec4 plane = camera.frustum[i];
        float dist = dot(plane.xyz, center) - plane.w;
        
        if(dist < -radius) return INTERSECTION_OUTSIDE;
        if(dist < radius) isIntersecting = true;
    }
    return isIntersecting ? INTERSECTION_INTERSECT : INTERSECTION_INSIDE;
}

uint TestAABBFrustum(vec3 aabbMin, vec3 aabbMax, CameraComponent camera) {
    vec3 extents = (aabbMax - aabbMin) * 0.5;
    vec3 center = (aabbMax + aabbMin) * 0.5;

    bool isIntersecting = false;
    for(int i = 0; i < 6; ++i) {
        vec4 plane = camera.frustum[i];
        float r = dot(extents, abs(plane.xyz));
        float dist = dot(plane.xyz, center) - plane.w;
        
        if (dist < -r) return INTERSECTION_OUTSIDE;
        if (dist < r) isIntersecting = true;
    }
    return isIntersecting ? INTERSECTION_INTERSECT : INTERSECTION_INSIDE;
}

bool TestSphereSphere(vec3 centerA, float radiusA, vec3 centerB, float radiusB) {
    vec3 diff = centerA - centerB;
    return dot(diff, diff) <= ((radiusA + radiusB) * (radiusA + radiusB));
}

bool TestAABBAABB(vec3 minA, vec3 maxA, vec3 minB, vec3 maxB) {
    return all(lessThanEqual(minA, maxB)) && all(greaterThanEqual(maxA, minB));
}

bool TestSphereAABB(vec3 sphereCenter, float sphereRadius, vec3 aabbMin, vec3 aabbMax) {
    vec3 closestPoint = clamp(sphereCenter, aabbMin, aabbMax);
    vec3 diff = closestPoint - sphereCenter;
    return dot(diff, diff) <= (sphereRadius * sphereRadius);
}

uint TestSphereFrustum(GpuMeshCollider collider, CameraComponent camera) {
    return TestSphereFrustum(collider.center, collider.radius, camera);
}

uint TestAABBFrustum(GpuMeshCollider collider, CameraComponent camera) {
    return TestAABBFrustum(collider.aabbMin, collider.aabbMax, camera);
}

uint TestFrustum(GpuMeshCollider collider, CameraComponent camera) {
    uint sphereResult = TestSphereFrustum(collider, camera);
    if (sphereResult != INTERSECTION_INTERSECT) return sphereResult;
    return TestAABBFrustum(collider, camera);
}

void TransformSphere(vec3 localCenter, float localRadius, mat4 transform, out vec3 worldCenter, out float worldRadius) {
    worldCenter = (transform * vec4(localCenter, 1.0)).xyz;
    vec3 scale = vec3(length(transform[0].xyz), length(transform[1].xyz), length(transform[2].xyz));
    worldRadius = localRadius * max(scale.x, max(scale.y, scale.z));
}

void TransformAABB(vec3 localMin, vec3 localMax, mat4 transform, out vec3 worldMin, out vec3 worldMax) {
    vec3 localExtents = (localMax - localMin) * 0.5;
    vec3 localCenter = (localMax + localMin) * 0.5;
    vec3 worldCenter = (transform * vec4(localCenter, 1.0)).xyz;
    
    mat3 absMatrix = mat3(abs(transform[0].xyz), abs(transform[1].xyz), abs(transform[2].xyz));
    vec3 worldExtents = absMatrix * localExtents;
   
    worldMin = worldCenter - worldExtents;
    worldMax = worldCenter + worldExtents;
}

void TransformCone(vec3 localApex, vec3 localAxis, float localCutoff, mat4 transform, mat4 transformIT, out vec3 worldApex, out vec3 worldAxis, out float worldCutoff) {
    worldApex = (transform * vec4(localApex, 1.0)).xyz;
    worldAxis = normalize((transformIT * vec4(localAxis, 0.0)).xyz);
    worldCutoff = localCutoff;
}

GpuMeshCollider TransformCollider(GpuMeshCollider local, mat4 transform) {
    GpuMeshCollider world;
    TransformSphere(local.center, local.radius, transform, world.center, world.radius);
    TransformAABB(local.aabbMin, local.aabbMax, transform, world.aabbMin, world.aabbMax);
    return world;
}

GpuMeshletCollider TransformCollider(GpuMeshletCollider local, mat4 transform, mat4 transformIT) {
    GpuMeshletCollider world;
    TransformSphere(local.center, local.radius, transform, world.center, world.radius);
    TransformAABB(local.aabbMin, local.aabbMax, transform, world.aabbMin, world.aabbMax);
    TransformCone(local.apex, local.axis, local.cutoff, transform, transformIT, world.apex, world.axis, world.cutoff);
    return world;
}

#endif