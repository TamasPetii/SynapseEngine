#pragma once
#include "Engine/SynMacro.h"
#include "Engine/Collision/Colliders/FrustumCollider.h"
#include "Engine/Mesh/Data/Gpu/GpuIndexedDrawData.h"
#include <glm/glm.hpp>
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

        SYN_INLINE static bool IsInFrustum(const glm::vec3& center, float radius, const glm::vec3& aabbMin, const glm::vec3& aabbMax, const FrustumCollider& frustum);
        SYN_INLINE static bool IsInFrustum(const GpuMeshCollider& collider, const FrustumCollider& frustum);
        SYN_INLINE static bool TestSphereFrustum(const GpuMeshCollider& collider, const FrustumCollider& frustum);
        SYN_INLINE static bool TestAabbFrustum(const GpuMeshCollider& collider, const FrustumCollider& frustum);

        SYN_INLINE static IntersectionType IsInFrustumIntersectionType(const glm::vec3& center, float radius, const glm::vec3& aabbMin, const glm::vec3& aabbMax, const FrustumCollider& frustum);
        SYN_INLINE static IntersectionType IsInFrustumIntersectionType(const GpuMeshCollider& collider, const FrustumCollider& frustum);
        SYN_INLINE static IntersectionType TestSphereFrustumIntersectionType(const GpuMeshCollider& collider, const FrustumCollider& frustum);
        SYN_INLINE static IntersectionType TestAabbFrustumIntersectionType(const GpuMeshCollider& collider, const FrustumCollider& frustum);
    private:
        SYN_INLINE static float GetSignedDistance(const glm::vec4& plane, const glm::vec3& point);
    };

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
}