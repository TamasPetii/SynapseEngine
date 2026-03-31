#include "CollisionTester.h"
#include "Engine/Mesh/Data/Gpu/GpuIndexedDrawData.h"
#include <glm/glm.hpp>

namespace Syn
{
    bool CollisionTester::TestSphereFrustum(const glm::vec3& center, float radius, std::span<const FrustumFace> frustum)
    {
        for (const auto& face : frustum)
        {
            if (!(GetSignedDistance(face, center) > -radius))
                return false;
        }
        return true;
    }

    bool CollisionTester::TestAabbFrustum(const glm::vec3& aabbMin, const glm::vec3& aabbMax, std::span<const FrustumFace> frustum)
    {
        glm::vec3 extents = (aabbMax - aabbMin) * 0.5f;
        glm::vec3 center = (aabbMax + aabbMin) * 0.5f;

        for (const auto& face : frustum)
        {
            float r = glm::dot(extents, glm::abs(face.normal));
            if (!(-r <= GetSignedDistance(face, center)))
                return false;
        }
        return true;
    }

    IntersectionType CollisionTester::TestSphereFrustumIntersectionType(const glm::vec3& center, float radius, std::span<const FrustumFace> frustum)
    {
        bool isIntersecting = false;
        for (const auto& face : frustum)
        {
            float distance = GetSignedDistance(face, center);
            if (distance < -radius) return IntersectionType::Outside;
            if (distance < radius) isIntersecting = true;
        }
        return isIntersecting ? IntersectionType::Intersect : IntersectionType::Inside;
    }

    IntersectionType CollisionTester::TestAabbFrustumIntersectionType(const glm::vec3& aabbMin, const glm::vec3& aabbMax, std::span<const FrustumFace> frustum)
    {
        glm::vec3 extents = (aabbMax - aabbMin) * 0.5f;
        glm::vec3 center = (aabbMax + aabbMin) * 0.5f;
        bool isIntersecting = false;

        for (const auto& face : frustum)
        {
            float r = glm::dot(extents, glm::abs(face.normal));
            float distance = GetSignedDistance(face, center);
            if (distance < -r) return IntersectionType::Outside;
            if (distance < r) isIntersecting = true;
        }
        return isIntersecting ? IntersectionType::Intersect : IntersectionType::Inside;
    }

    bool CollisionTester::IsInFrustum(const glm::vec3& center, float radius, const glm::vec3& aabbMin, const glm::vec3& aabbMax, std::span<const FrustumFace> frustum)
    {
        IntersectionType sphereResult = TestSphereFrustumIntersectionType(center, radius, frustum);

        if (sphereResult == IntersectionType::Outside)
            return false;

        if (sphereResult == IntersectionType::Inside)
            return true;

        return TestAabbFrustum(aabbMin, aabbMax, frustum);
    }

    IntersectionType CollisionTester::IsInFrustumIntersectionType(const glm::vec3& center, float radius, const glm::vec3& aabbMin, const glm::vec3& aabbMax, std::span<const FrustumFace> frustum)
    {
        IntersectionType sphereResult = TestSphereFrustumIntersectionType(center, radius, frustum);

        if (sphereResult != IntersectionType::Intersect)
            return sphereResult;

        return TestAabbFrustumIntersectionType(aabbMin, aabbMax, frustum);
    }

    bool CollisionTester::IsInFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum)
    {
        return IsInFrustum(collider.center, collider.radius, collider.aabbMin, collider.aabbMax, frustum);
    }

    bool CollisionTester::TestSphereFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum)
    {
        return TestSphereFrustum(collider.center, collider.radius, frustum);
    }

    bool CollisionTester::TestAabbFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum)
    {
        return TestAabbFrustum(collider.aabbMin, collider.aabbMax, frustum);
    }

    IntersectionType CollisionTester::IsInFrustumIntersectionType(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum)
    {
        return IsInFrustumIntersectionType(collider.center, collider.radius, collider.aabbMin, collider.aabbMax, frustum);
    }

    IntersectionType CollisionTester::TestSphereFrustumIntersectionType(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum)
    {
        return TestSphereFrustumIntersectionType(collider.center, collider.radius, frustum);
    }

    IntersectionType CollisionTester::TestAabbFrustumIntersectionType(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum)
    {
        return TestAabbFrustumIntersectionType(collider.aabbMin, collider.aabbMax, frustum);
    }

    float CollisionTester::GetSignedDistance(const FrustumFace& face, const glm::vec3& point)
    {
        return glm::dot(face.normal, point) - face.distance;
    }
}