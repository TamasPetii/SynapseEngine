#pragma once
#include "Engine/Component/CameraComponent.h"
#include <vector>
#include <span>

namespace Syn
{
    struct GpuMeshCollider;
    struct FrustumFace;

    class SYN_API CollisionTester
    {
    public:
        static bool IsInFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum);
        static bool TestSphereFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum);
        static bool TestAABBFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum);
    private:
        static float GetSignedDistance(const FrustumFace& face, const glm::vec3& point);
    };
}