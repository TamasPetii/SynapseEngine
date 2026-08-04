#pragma once
#include "Engine/Physics/IPhysicsEngine.h"
#include <box3d/box3d.h>
#include <vector>
#include <mutex> // Mutex hozzáadva a thread-safety miatt

namespace Syn
{
    class SYN_API Box3DPhysicsEngine final : public IPhysicsEngine
    {
    public:
        Box3DPhysicsEngine();
        ~Box3DPhysicsEngine() override;

        void Init(const PhysicsInitParams& params = {}) override;
        void Shutdown() override;
        void Update(float deltaTime) override;

        void SetBodyTransform(PhysicsBodyID bodyId, const glm::vec3& position, const glm::quat& rotation) override;
        void GetBodyTransform(PhysicsBodyID bodyId, glm::vec3& outPosition, glm::quat& outRotation) const override;
        bool IsBodyActive(PhysicsBodyID bodyId) const override;

        PhysicsBodyID CreateBoxBody(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& halfExtents, const PhysicsBodySettings& settings) override;
        PhysicsBodyID CreateSphereBody(const glm::vec3& position, const glm::quat& rotation, float radius, const PhysicsBodySettings& settings) override;
        PhysicsBodyID CreateCapsuleBody(const glm::vec3& position, const glm::quat& rotation, float halfHeight, float radius, const PhysicsBodySettings& settings) override;
        PhysicsBodyID CreateConvexBody(const glm::vec3& position, const glm::quat& rotation, std::span<const glm::vec3> vertices, const glm::vec3& scale, const PhysicsBodySettings& settings) override;
        PhysicsBodyID CreateMeshBody(const glm::vec3& position, const glm::quat& rotation, std::span<const glm::vec3> vertices, std::span<const uint32_t> indices, const glm::vec3& scale, const PhysicsBodySettings& settings) override;

        void DestroyBody(PhysicsBodyID bodyId) override;

        void SetBoxShape(PhysicsBodyID bodyId, const glm::vec3& newHalfExtents) override;
        void SetSphereShape(PhysicsBodyID bodyId, float newRadius) override;
        void SetCapsuleShape(PhysicsBodyID bodyId, float newHalfHeight, float newRadius) override;
        void SetConvexShape(PhysicsBodyID bodyId, std::span<const glm::vec3> newVertices, const glm::vec3& scale) override;
        void SetMeshShape(PhysicsBodyID bodyId, std::span<const glm::vec3> newVertices, std::span<const uint32_t> newIndices, const glm::vec3& scale) override;

        void SetBodyFriction(PhysicsBodyID bodyId, float friction) override;
        void SetBodyRestitution(PhysicsBodyID bodyId, float restitution) override;
        void SetBodyMotionType(PhysicsBodyID bodyId, PhysicsMotionType motionType) override;

        void GetDebugDrawData(PhysicsDrawData& outData) override;

    private:
        struct B3Entity {
            b3BodyId body = b3_nullBodyId;
            b3ShapeId shape = b3_nullShapeId;
            b3MeshData* meshData = nullptr;
            PhysicsBodySettings settings;
        };

        b3WorldId _worldId = {};

        std::vector<B3Entity> _bodyMap;
        std::vector<PhysicsBodyID> _freeSlots;

        mutable std::mutex _engineMutex;

        PhysicsBodyID AddEntity(b3BodyId b3Id, b3ShapeId b3Shape, const PhysicsBodySettings& settings, b3MeshData* meshData = nullptr);
        B3Entity GetEntity(PhysicsBodyID id) const;

        b3ShapeDef CreateShapeDef(const PhysicsBodySettings& settings) const;
        void ReplaceShape(PhysicsBodyID bodyId, b3ShapeId newShape, b3MeshData* newMeshData = nullptr);
        b3Quat GlmToB3Quat(const glm::quat& q) const;
    };
}