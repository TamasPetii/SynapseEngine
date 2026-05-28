#pragma once
#include "Engine/SynApi.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "PhysicsTypes.h"
#include <span>

namespace Syn
{
    class SYN_API IPhysicsEngine
    {
    public:
        virtual ~IPhysicsEngine() = default;

        virtual void Init(const PhysicsInitParams& params = {}) = 0;
        virtual void Shutdown() = 0;
        virtual void Update(float deltaTime) = 0;

        virtual void SetBodyTransform(PhysicsBodyID bodyId, const glm::vec3& position, const glm::quat& rotation) = 0;
        virtual void GetBodyTransform(PhysicsBodyID bodyId, glm::vec3& outPosition, glm::quat& outRotation) const = 0;
        virtual bool IsBodyActive(PhysicsBodyID bodyId) const = 0;

        virtual PhysicsBodyID CreateBoxBody(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& halfExtents, const PhysicsBodySettings& settings) = 0;
        virtual PhysicsBodyID CreateSphereBody(const glm::vec3& position, const glm::quat& rotation, float radius, const PhysicsBodySettings& settings) = 0;
        virtual PhysicsBodyID CreateCapsuleBody(const glm::vec3& position, const glm::quat& rotation, float halfHeight, float radius, const PhysicsBodySettings& settings) = 0;
        virtual PhysicsBodyID CreateConvexBody(const glm::vec3& position, const glm::quat& rotation, std::span<const glm::vec3> vertices, const glm::vec3& scale, const PhysicsBodySettings& settings) = 0;
        virtual PhysicsBodyID CreateMeshBody(const glm::vec3& position, const glm::quat& rotation, std::span<const glm::vec3> vertices, std::span<const uint32_t> indices, const glm::vec3& scale, const PhysicsBodySettings& settings) = 0;
        
        virtual void DestroyBody(PhysicsBodyID bodyId) = 0;
        virtual void SetBoxShape(PhysicsBodyID bodyId, const glm::vec3& newHalfExtents) = 0;
        virtual void SetSphereShape(PhysicsBodyID bodyId, float newRadius) = 0;
        virtual void SetCapsuleShape(PhysicsBodyID bodyId, float newHalfHeight, float newRadius) = 0;
        virtual void SetConvexShape(PhysicsBodyID bodyId, std::span<const glm::vec3> newVertices, const glm::vec3& scale) = 0;
        virtual void SetMeshShape(PhysicsBodyID bodyId, std::span<const glm::vec3> newVertices, std::span<const uint32_t> newIndices, const glm::vec3& scale) = 0;

        virtual void SetBodyFriction(PhysicsBodyID bodyId, float friction) = 0;
        virtual void SetBodyRestitution(PhysicsBodyID bodyId, float restitution) = 0;
        virtual void SetBodyMotionType(PhysicsBodyID bodyId, PhysicsMotionType motionType) = 0;
    };
}