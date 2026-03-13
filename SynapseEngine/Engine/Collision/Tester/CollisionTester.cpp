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
        for (const auto& plane : frustum)
        {
            float distance = glm::dot(plane.normal, collider.center) + plane.distance;

            if (distance < -collider.radius)
            {
                return false;
            }
        }
        return true;
    }

    bool CollisionTester::TestAABBFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum)
    {
        for (const auto& plane : frustum)
        {
            glm::vec3 positiveVertex = collider.aabbMin;
            if (plane.normal.x >= 0.0f) positiveVertex.x = collider.aabbMax.x;
            if (plane.normal.y >= 0.0f) positiveVertex.y = collider.aabbMax.y;
            if (plane.normal.z >= 0.0f) positiveVertex.z = collider.aabbMax.z;

            float distance = glm::dot(plane.normal, positiveVertex) + plane.distance;
            if (distance < 0.0f)
            {
                return false;
            }
        }
        return true;
    }
}