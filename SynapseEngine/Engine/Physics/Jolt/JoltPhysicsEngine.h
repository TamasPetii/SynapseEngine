#pragma once
#include "Engine/Physics/IPhysicsEngine.h"
#include <memory>
#include <span>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Core/JobSystemWithBarrier.h>

namespace Syn
{
    namespace Layers {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::ObjectLayer DEBRIS = 2;
        static constexpr JPH::ObjectLayer SENSOR = 3;
        static constexpr JPH::ObjectLayer NUM_LAYERS = 4;
    };

    namespace BroadPhaseLayers {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr JPH::uint NUM_LAYERS(2);
    };

    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
    public:
        BPLayerInterfaceImpl() {
            mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[Layers::SENSOR] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
            mObjectToBroadPhase[Layers::DEBRIS] = BroadPhaseLayers::MOVING;
        }

        virtual JPH::uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }
        virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override { return mObjectToBroadPhase[inLayer]; }
    private:
        JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
    };

    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
            switch (inLayer1) {
            case Layers::NON_MOVING: return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:     return true;
            case Layers::DEBRIS:     return true;
            case Layers::SENSOR:     return inLayer2 == BroadPhaseLayers::MOVING;
            default:                 return false;
            }
        }
    };

    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
            switch (inObject1) {
            case Layers::NON_MOVING: return inObject2 == Layers::MOVING || inObject2 == Layers::DEBRIS;
            case Layers::MOVING:     return true;
            case Layers::DEBRIS:     return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING;
            case Layers::SENSOR:     return inObject2 == Layers::MOVING;
            default:                 return false;
            }
        }
    };

    class SYN_API JoltPhysicsEngine : public IPhysicsEngine
    {
    public:
        JoltPhysicsEngine();
        ~JoltPhysicsEngine() override;

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
        PhysicsBodyID CreateBodyFromShape(JPH::ShapeRefC shape, const glm::vec3& position, const glm::quat& rotation, const PhysicsBodySettings& settings);
    private:
        std::unique_ptr<JPH::PhysicsSystem> physicsSystem;
        std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator;
        std::unique_ptr<JPH::JobSystem> jobSystem;

        BPLayerInterfaceImpl bpLayerInterface;
        ObjectVsBroadPhaseLayerFilterImpl objVsBpFilter;
        ObjectLayerPairFilterImpl objPairFilter;
    };
}