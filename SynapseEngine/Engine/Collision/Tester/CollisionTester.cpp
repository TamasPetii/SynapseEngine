#include "CollisionTester.h"
#include "Engine/Mesh/Data/Gpu/GpuIndexedDrawData.h"
#include <glm/glm.hpp>

namespace Syn
{
    bool CollisionTester::IsInFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum)
    {
        if (!TestSphereFrustum(collider, frustum))
            return false;

        return TestAABBFrustum(collider, frustum);
    }

    bool CollisionTester::TestSphereFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum)
    {
        for (const auto& face : frustum)
        {
            if (!(GetSignedDistance(face, collider.center) > -collider.radius))
                return false;
        }

        return true;
    }

    bool CollisionTester::TestAABBFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum)
    {
        glm::vec3 extents = (collider.aabbMax - collider.aabbMin) * 0.5f;
        glm::vec3 center = (collider.aabbMax + collider.aabbMin) * 0.5f;

        for (const auto& face : frustum)
        {
            float r = glm::dot(extents, glm::abs(face.normal));

            if (!(-r <= GetSignedDistance(face, center)))
                return false;
        }

        return true;
    }

    float CollisionTester::GetSignedDistance(const FrustumFace& face, const glm::vec3& point)
    {
        return glm::dot(face.normal, point) - face.distance;
    }

}