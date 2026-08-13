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

#ifndef SYN_INCLUDES_UTILS_CULLING_MATH_GLSL
#define SYN_INCLUDES_UTILS_CULLING_MATH_GLSL

#include "../Common/Camera.glsl"
#include "../Common/DirectionLight.glsl"
#include "../Common/Mesh.glsl"

#define INTERSECTION_OUTSIDE   0u
#define INTERSECTION_INTERSECT 1u
#define INTERSECTION_INSIDE    2u

// Cone Culling 

bool TestConeCulling(vec3 apex, vec3 axis, float cutoff, vec3 cameraEye) {
    vec3 view = normalize(apex - cameraEye);
    return dot(view, axis) >= cutoff;
}

bool TestConeCulling(vec3 axis, float cutoff, vec3 lightDir) {
    return dot(lightDir, axis) >= cutoff;
}

bool TestConeCulling(GpuMeshletCollider collider, vec3 cameraEye) {
    return TestConeCulling(collider.apex, collider.axis, collider.cutoff, cameraEye);
}

bool TestConeCullingLight(GpuMeshletCollider collider, vec3 lightDir) {
    return TestConeCulling(collider.axis, collider.cutoff, lightDir);
}

//Frustum Culling (Frustum, Aabb, Sphere, Cone)

uint TestSphereFrustum(vec3 center, float radius, vec4 planes[6]) {
    bool isIntersecting = false;
    for(int i = 0; i < 6; ++i) {
        vec4 plane = planes[i];
        float dist = dot(plane.xyz, center) - plane.w;
        
        if(dist < -radius) return INTERSECTION_OUTSIDE;
        if(dist < radius) isIntersecting = true;
    }
    return isIntersecting ? INTERSECTION_INTERSECT : INTERSECTION_INSIDE;
}

uint TestAABBFrustum(vec3 aabbMin, vec3 aabbMax, vec4 planes[6]) {
    vec3 extents = (aabbMax - aabbMin) * 0.5;
    vec3 center = (aabbMax + aabbMin) * 0.5;

    bool isIntersecting = false;
    for(int i = 0; i < 6; ++i) {
        vec4 plane = planes[i];
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

uint TestSphereSphereState(vec3 centerA, float radiusA, vec3 centerB, float radiusB) {
    vec3 diff = centerA - centerB;
    float distSq = dot(diff, diff);
    float radSum = radiusA + radiusB;
    
    if (distSq > radSum * radSum) return INTERSECTION_OUTSIDE;

    float radDiff = radiusA - radiusB;
    if (radDiff >= 0.0 && distSq <= radDiff * radDiff) {
        return INTERSECTION_INSIDE;
    }
    return INTERSECTION_INTERSECT;
}

bool TestAABBAABB(vec3 minA, vec3 maxA, vec3 minB, vec3 maxB) {
    return all(lessThanEqual(minA, maxB)) && all(greaterThanEqual(maxA, minB));
}

float GetMinAbs(float minVal, float maxVal) {
    if (minVal <= 0.0 && maxVal >= 0.0) return 0.0;
    return min(abs(minVal), abs(maxVal));
}

uint GetPointLightFaceVisibilityMask(vec3 modelCenter, float modelRadius, vec3 lightCenter) {
    vec3 relMin = (modelCenter - vec3(modelRadius)) - lightCenter;
    vec3 relMax = (modelCenter + vec3(modelRadius)) - lightCenter;

    float minAbsX = GetMinAbs(relMin.x, relMax.x);
    float minAbsY = GetMinAbs(relMin.y, relMax.y);
    float minAbsZ = GetMinAbs(relMin.z, relMax.z);

    uint faceMask = 0;
    if (relMax.x > 0.0 && relMax.x >= minAbsY && relMax.x >= minAbsZ) faceMask |= (1u << 0); // +X
    if (relMin.x < 0.0 && -relMin.x >= minAbsY && -relMin.x >= minAbsZ) faceMask |= (1u << 1); // -X
    if (relMax.y > 0.0 && relMax.y >= minAbsX && relMax.y >= minAbsZ) faceMask |= (1u << 2); // +Y
    if (relMin.y < 0.0 && -relMin.y >= minAbsX && -relMin.y >= minAbsZ) faceMask |= (1u << 3); // -Y
    if (relMax.z > 0.0 && relMax.z >= minAbsX && relMax.z >= minAbsY) faceMask |= (1u << 4); // +Z
    if (relMin.z < 0.0 && -relMin.z >= minAbsX && -relMin.z >= minAbsY) faceMask |= (1u << 5); // -Z

    return faceMask;
}

bool TestSphereAABB(vec3 sphereCenter, float sphereRadius, vec3 aabbMin, vec3 aabbMax) {
    vec3 closestPoint = clamp(sphereCenter, aabbMin, aabbMax);
    vec3 diff = closestPoint - sphereCenter;
    return dot(diff, diff) <= (sphereRadius * sphereRadius);
}

uint TestSphereAABBState(vec3 sphereCenter, float sphereRadius, vec3 aabbMin, vec3 aabbMax) {
    vec3 closestPoint = clamp(sphereCenter, aabbMin, aabbMax);
    vec3 diffClosest = sphereCenter - closestPoint;
    if (dot(diffClosest, diffClosest) > sphereRadius * sphereRadius) {
        return INTERSECTION_OUTSIDE;
    }

    vec3 aabbCenter = (aabbMin + aabbMax) * 0.5;
    vec3 furthestPoint = vec3(
        (sphereCenter.x < aabbCenter.x) ? aabbMax.x : aabbMin.x,
        (sphereCenter.y < aabbCenter.y) ? aabbMax.y : aabbMin.y,
        (sphereCenter.z < aabbCenter.z) ? aabbMax.z : aabbMin.z
    );

    vec3 diffFurthest = sphereCenter - furthestPoint;
    if (dot(diffFurthest, diffFurthest) <= sphereRadius * sphereRadius) {
        return INTERSECTION_INSIDE;
    }

    return INTERSECTION_INTERSECT;
}

uint TestSphere(GpuMeshCollider collider, vec3 sphereCenter, float sphereRadius) {
    uint sphereResult = TestSphereSphereState(sphereCenter, sphereRadius, collider.center, collider.radius);
    
    if (sphereResult != INTERSECTION_INTERSECT) return sphereResult;
    
    return TestSphereAABBState(sphereCenter, sphereRadius, collider.aabbMin, collider.aabbMax);
}

uint TestSphereFrustum(GpuMeshCollider collider, vec4 planes[6]) {
    return TestSphereFrustum(collider.center, collider.radius, planes);
}

uint TestAABBFrustum(GpuMeshCollider collider, vec4 planes[6]) {
    return TestAABBFrustum(collider.aabbMin, collider.aabbMax, planes);
}

uint TestFrustum(GpuMeshCollider collider, vec4 planes[6]) {
    uint sphereResult = TestSphereFrustum(collider, planes);
    if (sphereResult != INTERSECTION_INTERSECT) return sphereResult;
    return TestAABBFrustum(collider, planes);
}

uint TestFrustum(vec3 center, float radius, vec3 aabbMin, vec3 aabbMax, vec4 planes[6]) {
    uint sphereResult = TestSphereFrustum(center, radius, planes);
    if (sphereResult != INTERSECTION_INTERSECT) return sphereResult;
    
    return TestAABBFrustum(aabbMin, aabbMax, planes);
}

//Paper: https://bartwronski.com/2017/04/13/cull-that-cone/
bool TestConeSphere(vec3 conePos, vec3 coneDir, float coneRange, float coneCosAngle, float coneSinAngle, vec3 sphereCenter, float sphereRadius) {
    vec3 v = sphereCenter - conePos;
    float lenSq = dot(v, v);
    float v1Len = dot(v, coneDir);

    float distanceClosestPoint = coneCosAngle * sqrt(max(lenSq - v1Len * v1Len, 0.0)) - v1Len * coneSinAngle;

    bool angleCull = distanceClosestPoint > sphereRadius;
    bool frontCull = v1Len > sphereRadius + coneRange;
    bool backCull  = v1Len < -sphereRadius;

    return !(angleCull || frontCull || backCull);
}

uint TestConeSphereState(vec3 conePos, vec3 coneDir, float coneRange, float coneCosAngle, float coneSinAngle, vec3 sphereCenter, float sphereRadius) {
    vec3 v = sphereCenter - conePos;
    float lenSq = dot(v, v);
    float v1Len = dot(v, coneDir);
    float distanceClosestPoint = coneCosAngle * sqrt(max(lenSq - v1Len * v1Len, 0.0)) - v1Len * coneSinAngle;

    if (distanceClosestPoint > sphereRadius || v1Len > sphereRadius + coneRange || v1Len < -sphereRadius) {
        return INTERSECTION_OUTSIDE;
    }

    bool fullyInAngle = distanceClosestPoint < -sphereRadius;
    bool fullyInFront = v1Len > sphereRadius;
    bool fullyBehindRange = v1Len < coneRange - sphereRadius;

    if (fullyInAngle && fullyInFront && fullyBehindRange) {
        return INTERSECTION_INSIDE;
    }

    return INTERSECTION_INTERSECT;
}

//Transform Collider

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

// Camera Wrapper Overloads 

uint TestSphereFrustum(vec3 center, float radius, CameraComponent camera) {
    return TestSphereFrustum(center, radius, camera.frustum);
}

uint TestAABBFrustum(vec3 aabbMin, vec3 aabbMax, CameraComponent camera) {
    return TestAABBFrustum(aabbMin, aabbMax, camera.frustum);
}

uint TestFrustum(vec3 center, float radius, vec3 aabbMin, vec3 aabbMax, CameraComponent camera) {
    return TestFrustum(center, radius, aabbMin, aabbMax, camera.frustum);
}

uint TestSphereFrustum(GpuMeshCollider collider, CameraComponent camera) {
    return TestSphereFrustum(collider.center, collider.radius, camera.frustum);
}

uint TestAABBFrustum(GpuMeshCollider collider, CameraComponent camera) {
    return TestAABBFrustum(collider.aabbMin, collider.aabbMax, camera.frustum);
}

uint TestFrustum(GpuMeshCollider collider, CameraComponent camera) {
    uint sphereResult = TestSphereFrustum(collider.center, collider.radius, camera.frustum);
    if (sphereResult != INTERSECTION_INTERSECT) return sphereResult;
    return TestAABBFrustum(collider.aabbMin, collider.aabbMax, camera.frustum);
}

// Dirlight Wrapper Overloads 

uint TestSphereFrustum(vec3 center, float radius, CascadeCollider cascade) {
    return TestSphereFrustum(center, radius, cascade.planes);
}

uint TestAABBFrustum(vec3 aabbMin, vec3 aabbMax, CascadeCollider cascade) {
    return TestAABBFrustum(aabbMin, aabbMax, cascade.planes);
}

uint TestFrustum(vec3 center, float radius, vec3 aabbMin, vec3 aabbMax, CascadeCollider cascade) {
    return TestFrustum(center, radius, aabbMin, aabbMax, cascade.planes);
}

uint TestSphereFrustum(GpuMeshCollider collider, CascadeCollider cascade) {
    return TestSphereFrustum(collider.center, collider.radius, cascade.planes);
}

uint TestAABBFrustum(GpuMeshCollider collider, CascadeCollider cascade) {
    return TestAABBFrustum(collider.aabbMin, collider.aabbMax, cascade.planes);
}

uint TestFrustum(GpuMeshCollider collider, CascadeCollider cascade) {
    uint sphereResult = TestSphereFrustum(collider.center, collider.radius, cascade.planes);
    if (sphereResult != INTERSECTION_INTERSECT) return sphereResult;
    return TestAABBFrustum(collider.aabbMin, collider.aabbMax, cascade.planes);
}

#endif