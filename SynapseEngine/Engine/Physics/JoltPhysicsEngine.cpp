#include "JoltPhysicsEngine.h"

namespace Syn
{
    JoltPhysicsEngine::JoltPhysicsEngine() {}

    JoltPhysicsEngine::~JoltPhysicsEngine() = default;

    void JoltPhysicsEngine::Init(const PhysicsInitParams& params)
    {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(params.tempAllocatorSizeMB * 1024 * 1024);

        jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
            JPH::cMaxPhysicsJobs,
            JPH::cMaxPhysicsBarriers,
            std::thread::hardware_concurrency() - 1
        );

        physicsSystem = std::make_unique<JPH::PhysicsSystem>();

        physicsSystem->Init(
            params.maxBodies,
            params.numBodyMutexes,
            params.maxBodyPairs,
            params.maxContactConstraints,
            bpLayerInterface,
            objVsBpFilter,
            objPairFilter
        );
    }

    void JoltPhysicsEngine::Shutdown()
    {
        physicsSystem.reset();
        jobSystem.reset();
        tempAllocator.reset();

        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }

    void JoltPhysicsEngine::Update(float deltaTime)
    {
        const int collisionSteps = 1;
        physicsSystem->Update(deltaTime, collisionSteps, tempAllocator.get(), jobSystem.get());
    }

    void JoltPhysicsEngine::SetBodyTransform(PhysicsBodyID bodyId, const glm::vec3& position, const glm::quat& rotation)
    {
        JPH::BodyID joltId(bodyId);
        JPH::Vec3 jPos(position.x, position.y, position.z);
        JPH::Quat jRot(rotation.x, rotation.y, rotation.z, rotation.w); // GLM XYZW, Jolt is XYZW

        physicsSystem->GetBodyInterface().SetPositionAndRotation(joltId, jPos, jRot, JPH::EActivation::Activate);
    }

    void JoltPhysicsEngine::GetBodyTransform(PhysicsBodyID bodyId, glm::vec3& outPosition, glm::quat& outRotation) const
    {
        JPH::BodyID joltId(bodyId);
        JPH::Vec3 jPos;
        JPH::Quat jRot;

        physicsSystem->GetBodyInterface().GetPositionAndRotation(joltId, jPos, jRot);

        outPosition = glm::vec3(jPos.GetX(), jPos.GetY(), jPos.GetZ());
        outRotation = glm::quat(jRot.GetW(), jRot.GetX(), jRot.GetY(), jRot.GetZ()); // GLM quat konstruktor: WXYZ!
    }

    bool JoltPhysicsEngine::IsBodyActive(PhysicsBodyID bodyId) const
    {
        return physicsSystem->GetBodyInterface().IsActive(JPH::BodyID(bodyId));
    }
}