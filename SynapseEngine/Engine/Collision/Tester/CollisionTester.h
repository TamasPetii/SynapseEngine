#pragma once
#include "Engine/SynMacro.h"
#include "Engine/Collision/Colliders/FrustumCollider.h"
#include "Engine/Mesh/Data/Gpu/GpuIndexedDrawData.h"
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>

namespace Syn
{
    struct GpuMeshCollider;

    enum class IntersectionType
    {
        Outside,
        Intersect,
        Inside
    };

    class SYN_API CollisionTester
    {
    public:
        SYN_INLINE static bool TestSphereFrustum(const glm::vec3& center, float radius, const FrustumCollider& frustum);
        SYN_INLINE static bool TestAabbFrustum(const glm::vec3& aabbMin, const glm::vec3& aabbMax, const FrustumCollider& frustum);
        SYN_INLINE static IntersectionType TestSphereFrustumIntersectionType(const glm::vec3& center, float radius, const FrustumCollider& frustum);
        SYN_INLINE static IntersectionType TestAabbFrustumIntersectionType(const glm::vec3& aabbMin, const glm::vec3& aabbMax, const FrustumCollider& frustum);

        SYN_INLINE static bool TestSphereSphere(const glm::vec3& centerA, float radiusA, const glm::vec3& centerB, float radiusB);
        SYN_INLINE static IntersectionType TestSphereSphereIntersectionType(const glm::vec3& centerA, float radiusA, const glm::vec3& centerB, float radiusB);

        SYN_INLINE static bool TestSphereAabb(const glm::vec3& sphereCenter, float sphereRadius, const glm::vec3& aabbMin, const glm::vec3& aabbMax);
        SYN_INLINE static IntersectionType TestSphereAabbIntersectionType(const glm::vec3& sphereCenter, float sphereRadius, const glm::vec3& aabbMin, const glm::vec3& aabbMax);

        SYN_INLINE static bool IsInFrustum(const glm::vec3& center, float radius, const glm::vec3& aabbMin, const glm::vec3& aabbMax, const FrustumCollider& frustum);
        SYN_INLINE static bool IsInFrustum(const GpuMeshCollider& collider, const FrustumCollider& frustum);
        SYN_INLINE static bool TestSphereFrustum(const GpuMeshCollider& collider, const FrustumCollider& frustum);
        SYN_INLINE static bool TestAabbFrustum(const GpuMeshCollider& collider, const FrustumCollider& frustum);

        SYN_INLINE static IntersectionType IsInFrustumIntersectionType(const glm::vec3& center, float radius, const glm::vec3& aabbMin, const glm::vec3& aabbMax, const FrustumCollider& frustum);
        SYN_INLINE static IntersectionType IsInFrustumIntersectionType(const GpuMeshCollider& collider, const FrustumCollider& frustum);
        SYN_INLINE static IntersectionType TestSphereFrustumIntersectionType(const GpuMeshCollider& collider, const FrustumCollider& frustum);
        SYN_INLINE static IntersectionType TestAabbFrustumIntersectionType(const GpuMeshCollider& collider, const FrustumCollider& frustum);
    
        SYN_INLINE static float CalculateSphereScreenSize(const glm::vec3& center, float radius, const glm::mat4& view, const glm::mat4& proj, float nearZ, const glm::vec2& screenRes);
        SYN_INLINE static uint32_t CalculateLodFromScreenSize(float screenSizePixels);

        SYN_INLINE static bool TestConeSphere(const glm::vec3& conePos, const glm::vec3& coneDir, float coneRange, float coneCosAngle, float coneSinAngle, const glm::vec3& sphereCenter, float sphereRadius);
        SYN_INLINE static IntersectionType TestConeSphereIntersectionType(const glm::vec3& conePos, const glm::vec3& coneDir, float coneRange, float coneCosAngle, float coneSinAngle, const glm::vec3& sphereCenter, float sphereRadius);
    
        SYN_INLINE static bool IsInSphere(const GpuMeshCollider& collider, const glm::vec3& sphereCenter, float sphereRadius);
        SYN_INLINE static IntersectionType IsInSphereIntersectionType(const GpuMeshCollider& collider, const glm::vec3& sphereCenter, float sphereRadius);
    private:
        SYN_INLINE static float GetSignedDistance(const glm::vec4& plane, const glm::vec3& point);
    };

    SYN_INLINE bool CollisionTester::TestSphereSphere(const glm::vec3& centerA, float radiusA, const glm::vec3& centerB, float radiusB)
    {
        float distSq = glm::distance2(centerA, centerB);
        float radSum = radiusA + radiusB;
        return distSq <= (radSum * radSum);
    }

    SYN_INLINE IntersectionType CollisionTester::TestSphereSphereIntersectionType(const glm::vec3& centerA, float radiusA, const glm::vec3& centerB, float radiusB)
    {
        float distSq = glm::distance2(centerA, centerB);
        float radSum = radiusA + radiusB;
        if (distSq > radSum * radSum) return IntersectionType::Outside;

        float radDiff = radiusA - radiusB;
        if (radDiff >= 0.0f && distSq <= radDiff * radDiff) {
            return IntersectionType::Inside;
        }
        return IntersectionType::Intersect;
    }

    SYN_INLINE bool CollisionTester::TestSphereAabb(const glm::vec3& sphereCenter, float sphereRadius, const glm::vec3& aabbMin, const glm::vec3& aabbMax)
    {
        glm::vec3 closestPoint = glm::clamp(sphereCenter, aabbMin, aabbMax);
        float distSq = glm::distance2(sphereCenter, closestPoint);
        return distSq <= (sphereRadius * sphereRadius);
    }

    SYN_INLINE IntersectionType CollisionTester::TestSphereAabbIntersectionType(const glm::vec3& sphereCenter, float sphereRadius, const glm::vec3& aabbMin, const glm::vec3& aabbMax)
    {
        glm::vec3 closestPoint = glm::clamp(sphereCenter, aabbMin, aabbMax);
        if (glm::distance2(sphereCenter, closestPoint) > sphereRadius * sphereRadius) {
            return IntersectionType::Outside;
        }

        glm::vec3 furthestPoint(
            (sphereCenter.x < (aabbMin.x + aabbMax.x) * 0.5f) ? aabbMax.x : aabbMin.x,
            (sphereCenter.y < (aabbMin.y + aabbMax.y) * 0.5f) ? aabbMax.y : aabbMin.y,
            (sphereCenter.z < (aabbMin.z + aabbMax.z) * 0.5f) ? aabbMax.z : aabbMin.z
        );

        if (glm::distance2(sphereCenter, furthestPoint) <= sphereRadius * sphereRadius) {
            return IntersectionType::Inside;
        }

        return IntersectionType::Intersect;
    }

    SYN_INLINE bool CollisionTester::TestSphereFrustum(const glm::vec3& center, float radius, const FrustumCollider& frustum)
    {
        for (int i = 0; i < 6; ++i)
        {
            if (GetSignedDistance(frustum.planes[i], center) < -radius)
                return false;
        }
        return true;
    }

    SYN_INLINE bool CollisionTester::TestAabbFrustum(const glm::vec3& aabbMin, const glm::vec3& aabbMax, const FrustumCollider& frustum)
    {
        glm::vec3 extents = (aabbMax - aabbMin) * 0.5f;
        glm::vec3 center = (aabbMax + aabbMin) * 0.5f;

        for (int i = 0; i < 6; ++i)
        {
            const glm::vec4& plane = frustum.planes[i];
            float r = glm::dot(extents, glm::abs(glm::vec3(plane)));
            if (GetSignedDistance(plane, center) < -r)
                return false;
        }
        return true;
    }

    SYN_INLINE IntersectionType CollisionTester::TestSphereFrustumIntersectionType(const glm::vec3& center, float radius, const FrustumCollider& frustum)
    {
        bool isIntersecting = false;
        for (int i = 0; i < 6; ++i)
        {
            float distance = GetSignedDistance(frustum.planes[i], center);
            if (distance < -radius) return IntersectionType::Outside;
            if (distance < radius) isIntersecting = true;
        }
        return isIntersecting ? IntersectionType::Intersect : IntersectionType::Inside;
    }

    SYN_INLINE IntersectionType CollisionTester::TestAabbFrustumIntersectionType(const glm::vec3& aabbMin, const glm::vec3& aabbMax, const FrustumCollider& frustum)
    {
        glm::vec3 extents = (aabbMax - aabbMin) * 0.5f;
        glm::vec3 center = (aabbMax + aabbMin) * 0.5f;
        bool isIntersecting = false;

        for (int i = 0; i < 6; ++i)
        {
            const glm::vec4& plane = frustum.planes[i];
            float r = glm::dot(extents, glm::abs(glm::vec3(plane)));
            float distance = GetSignedDistance(plane, center);
            if (distance < -r) return IntersectionType::Outside;
            if (distance < r) isIntersecting = true;
        }
        return isIntersecting ? IntersectionType::Intersect : IntersectionType::Inside;
    }

    SYN_INLINE bool CollisionTester::IsInFrustum(const glm::vec3& center, float radius, const glm::vec3& aabbMin, const glm::vec3& aabbMax, const FrustumCollider& frustum)
    {
        IntersectionType sphereResult = TestSphereFrustumIntersectionType(center, radius, frustum);

        if (sphereResult == IntersectionType::Outside)
            return false;

        if (sphereResult == IntersectionType::Inside)
            return true;

        return TestAabbFrustum(aabbMin, aabbMax, frustum);
    }

    SYN_INLINE IntersectionType CollisionTester::IsInFrustumIntersectionType(const glm::vec3& center, float radius, const glm::vec3& aabbMin, const glm::vec3& aabbMax, const FrustumCollider& frustum)
    {
        IntersectionType sphereResult = TestSphereFrustumIntersectionType(center, radius, frustum);

        if (sphereResult != IntersectionType::Intersect)
            return sphereResult;

        return TestAabbFrustumIntersectionType(aabbMin, aabbMax, frustum);
    }

    SYN_INLINE bool CollisionTester::IsInFrustum(const GpuMeshCollider& collider, const FrustumCollider& frustum)
    {
        return IsInFrustum(collider.center, collider.radius, collider.aabbMin, collider.aabbMax, frustum);
    }

    SYN_INLINE bool CollisionTester::TestSphereFrustum(const GpuMeshCollider& collider, const FrustumCollider& frustum)
    {
        return TestSphereFrustum(collider.center, collider.radius, frustum);
    }

    SYN_INLINE bool CollisionTester::TestAabbFrustum(const GpuMeshCollider& collider, const FrustumCollider& frustum)
    {
        return TestAabbFrustum(collider.aabbMin, collider.aabbMax, frustum);
    }

    SYN_INLINE IntersectionType CollisionTester::IsInFrustumIntersectionType(const GpuMeshCollider& collider, const FrustumCollider& frustum)
    {
        return IsInFrustumIntersectionType(collider.center, collider.radius, collider.aabbMin, collider.aabbMax, frustum);
    }

    SYN_INLINE IntersectionType CollisionTester::TestSphereFrustumIntersectionType(const GpuMeshCollider& collider, const FrustumCollider& frustum)
    {
        return TestSphereFrustumIntersectionType(collider.center, collider.radius, frustum);
    }

    SYN_INLINE IntersectionType CollisionTester::TestAabbFrustumIntersectionType(const GpuMeshCollider& collider, const FrustumCollider& frustum)
    {
        return TestAabbFrustumIntersectionType(collider.aabbMin, collider.aabbMax, frustum);
    }

    SYN_INLINE float CollisionTester::GetSignedDistance(const glm::vec4& plane, const glm::vec3& point)
    {
        return glm::dot(glm::vec3(plane), point) - plane.w;
    }

    SYN_INLINE float CollisionTester::CalculateSphereScreenSize(const glm::vec3& center, float radius, const glm::mat4& view, const glm::mat4& proj, float nearZ, const glm::vec2& screenRes)
    {
        glm::vec3 viewCenter = glm::vec3(view * glm::vec4(center, 1.0f));

        if (-viewCenter.z - radius < nearZ) return 99999.0f;

        glm::vec2 cx(viewCenter.x, -viewCenter.z);
        float cx2 = glm::dot(cx, cx);
        float r2 = radius * radius;

        if (cx2 < r2) return 99999.0f;

        glm::vec2 vx(std::sqrt(cx2 - r2), radius);
        glm::vec2 minx = glm::mat2(vx.x, vx.y, -vx.y, vx.x) * cx;
        glm::vec2 maxx = glm::mat2(vx.x, -vx.y, vx.y, vx.x) * cx;

        glm::vec2 cy(viewCenter.y, -viewCenter.z);
        float cy2 = glm::dot(cy, cy);

        if (cy2 < r2) return 99999.0f;

        glm::vec2 vy(std::sqrt(cy2 - r2), radius);
        glm::vec2 miny = glm::mat2(vy.x, vy.y, -vy.y, vy.x) * cy;
        glm::vec2 maxy = glm::mat2(vy.x, -vy.y, vy.y, vy.x) * cy;

        float p00 = std::abs(proj[0][0]);
        float p11 = std::abs(proj[1][1]);

        glm::vec4 uvBounds(
            minx.x / minx.y * p00,
            miny.x / miny.y * p11,
            maxx.x / maxx.y * p00,
            maxy.x / maxy.y * p11
        );

        uvBounds = uvBounds * 0.5f + 0.5f;

        glm::vec2 sizeInPixels = (glm::vec2(uvBounds.z, uvBounds.w) - glm::vec2(uvBounds.x, uvBounds.y)) * screenRes;

        return std::max(sizeInPixels.x, sizeInPixels.y);
    }

    SYN_INLINE uint32_t CollisionTester::CalculateLodFromScreenSize(float screenSizePixels)
    {
        if (screenSizePixels > 512.0f) return 0;
        if (screenSizePixels > 256.0f) return 1;
        if (screenSizePixels > 128.0f) return 2;
        return 3;
    }

    SYN_INLINE bool CollisionTester::TestConeSphere(const glm::vec3& conePos, const glm::vec3& coneDir, float coneRange, float coneCosAngle, float coneSinAngle, const glm::vec3& sphereCenter, float sphereRadius)
    {
        glm::vec3 v = sphereCenter - conePos;
        float lenSq = glm::dot(v, v);
        float v1Len = glm::dot(v, coneDir);
        float distanceClosestPoint = coneCosAngle * std::sqrt(std::max(lenSq - v1Len * v1Len, 0.0f)) - v1Len * coneSinAngle;

        bool angleCull = distanceClosestPoint > sphereRadius;
        bool frontCull = v1Len > sphereRadius + coneRange;
        bool backCull = v1Len < -sphereRadius;

        return !(angleCull || frontCull || backCull);
    }

    SYN_INLINE IntersectionType CollisionTester::TestConeSphereIntersectionType(const glm::vec3& conePos, const glm::vec3& coneDir, float coneRange, float coneCosAngle, float coneSinAngle, const glm::vec3& sphereCenter, float sphereRadius)
    {
        glm::vec3 v = sphereCenter - conePos;
        float lenSq = glm::dot(v, v);
        float v1Len = glm::dot(v, coneDir);
        float distanceClosestPoint = coneCosAngle * std::sqrt(std::max(lenSq - v1Len * v1Len, 0.0f)) - v1Len * coneSinAngle;

        if (distanceClosestPoint > sphereRadius || v1Len > sphereRadius + coneRange || v1Len < -sphereRadius) {
            return IntersectionType::Outside;
        }

        bool fullyInAngle = distanceClosestPoint < -sphereRadius;
        bool fullyInFront = v1Len > sphereRadius;
        bool fullyBehindRange = v1Len < coneRange - sphereRadius;

        if (fullyInAngle && fullyInFront && fullyBehindRange) {
            return IntersectionType::Inside;
        }

        return IntersectionType::Intersect;
    }

    SYN_INLINE bool CollisionTester::IsInSphere(const GpuMeshCollider& collider, const glm::vec3& sphereCenter, float sphereRadius)
    {
        if (!TestSphereSphere(sphereCenter, sphereRadius, collider.center, collider.radius))
            return false;

        return TestSphereAabb(sphereCenter, sphereRadius, collider.aabbMin, collider.aabbMax);
    }

    SYN_INLINE IntersectionType CollisionTester::IsInSphereIntersectionType(const GpuMeshCollider& collider, const glm::vec3& sphereCenter, float sphereRadius)
    {
        IntersectionType sphereResult = TestSphereSphereIntersectionType(sphereCenter, sphereRadius, collider.center, collider.radius);

        if (sphereResult != IntersectionType::Intersect)
            return sphereResult;

        return TestSphereAabbIntersectionType(sphereCenter, sphereRadius, collider.aabbMin, collider.aabbMax);
    }
}