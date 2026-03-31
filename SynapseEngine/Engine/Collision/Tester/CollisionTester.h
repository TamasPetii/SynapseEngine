#pragma once
#include "Engine/SynMacro.h"
#include "Engine/Component/CameraComponent.h"
#include <vector>
#include <span>

namespace Syn
{
    struct GpuMeshCollider;
    struct FrustumFace;

    enum class IntersectionType
    {
        Outside,
        Intersect,
        Inside
    };

    class SYN_API CollisionTester
    {
    public:
        SYN_INLINE static bool TestSphereFrustum(const glm::vec3& center, float radius, std::span<const FrustumFace> frustum);
        SYN_INLINE static bool TestAabbFrustum(const glm::vec3& aabbMin, const glm::vec3& aabbMax, std::span<const FrustumFace> frustum);
        SYN_INLINE static IntersectionType TestSphereFrustumIntersectionType(const glm::vec3& center, float radius, std::span<const FrustumFace> frustum);
        SYN_INLINE static IntersectionType TestAabbFrustumIntersectionType(const glm::vec3& aabbMin, const glm::vec3& aabbMax, std::span<const FrustumFace> frustum);

        SYN_INLINE static bool IsInFrustum(const glm::vec3& center, float radius, const glm::vec3& aabbMin, const glm::vec3& aabbMax, std::span<const FrustumFace> frustum);
        SYN_INLINE static bool IsInFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum);
        SYN_INLINE static bool TestSphereFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum);
        SYN_INLINE static bool TestAabbFrustum(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum);

        SYN_INLINE static IntersectionType IsInFrustumIntersectionType(const glm::vec3& center, float radius, const glm::vec3& aabbMin, const glm::vec3& aabbMax, std::span<const FrustumFace> frustum);
        SYN_INLINE static IntersectionType IsInFrustumIntersectionType(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum);
        SYN_INLINE static IntersectionType TestSphereFrustumIntersectionType(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum);
        SYN_INLINE static IntersectionType TestAabbFrustumIntersectionType(const GpuMeshCollider& collider, std::span<const FrustumFace> frustum);
    private:
        SYN_INLINE static float GetSignedDistance(const FrustumFace& face, const glm::vec3& point);
    };
}