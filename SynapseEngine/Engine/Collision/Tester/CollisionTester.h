#pragma once
#include "Engine/Component/CameraComponent.h"
#include <vector>
#include <span>

namespace Syn
{
    struct GpuMeshCollider;

    class SYN_API CollisionTester
    {
    public:
        static bool IsInFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum);
        static bool TestSphereFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum);
        static bool TestAABBFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum);
    };
}