#include "PhysicsSystem.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Physics/RigidBodyComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Physics/IPhysicsEngine.h"

#include "Engine/System/Core/TransformSystem.h"
#include "Engine/System/Physics/RigidBodySystem.h"
#include "Engine/System/Physics/PhysicsDebugSystem.h"

namespace Syn
{
    std::vector<TypeID> PhysicsSystem::GetReadDependencies() const {
        return { 
            TypeInfo<PhysicsDebugSystem>::ID,
            TypeInfo<RigidBodySystem>::ID
        };
    }

    std::vector<TypeID> PhysicsSystem::GetWriteDependencies() const {
        return { TypeInfo<TransformSystem>::ID };
    }

    void PhysicsSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto rbPool = registry->GetPool<RigidBodyComponent>();

        IPhysicsEngine* physicsEngine = scene->GetPhysicsEngine();

        if (!transformPool || !rbPool || !physicsEngine) return;

        //Todo: Hmm static/dynamic entity handle?

        auto entities = rbPool->GetStorage().GetDenseEntities();

        std::optional<tf::Task> preUpdateTask = this->ForEach(entities, subflow, "SyncKinematicsToPhysics",
            [transformPool, rbPool, physicsEngine](EntityID entity) {
                if (transformPool->Has(entity)) { //Todo: Flag or static?
                    auto& rb = rbPool->Get(entity);
                    if (rb.motionType == PhysicsMotionType::Kinematic && rb.bodyID != INVALID_BODY_ID) {
                        auto& tr = transformPool->Get(entity);
                        glm::quat rotQuat(glm::radians(tr.rotation));
                        physicsEngine->SetBodyTransform(rb.bodyID, tr.translation, rotQuat);
                    }
                }
            });

        tf::Task simulateTask = this->EmplaceTask(subflow, "SimulatePhysics", [physicsEngine, deltaTime]() {

            const float maxDeltaTime = 1.0f / 15.0f;
            float safeDeltaTime = std::min(deltaTime, maxDeltaTime);

            physicsEngine->Update(safeDeltaTime);
            });

        std::optional<tf::Task> postUpdateTask = this->ForEach(entities, subflow, "SyncDynamicsToECS",
            [transformPool, rbPool, physicsEngine](EntityID entity) {
                if (transformPool->Has(entity)) {
                    auto& rb = rbPool->Get(entity);

                    if (!(rb.motionType == PhysicsMotionType::Kinematic) && rb.bodyID != INVALID_BODY_ID) {
                        if (physicsEngine->IsBodyActive(rb.bodyID)) {
                            auto& tr = transformPool->Get(entity);

                            glm::quat newRot;
                            physicsEngine->GetBodyTransform(rb.bodyID, tr.translation, newRot);
                            tr.rotation = glm::degrees(glm::eulerAngles(newRot));

                            transformPool->SetBit<CHANGED_BIT>(entity);
                            transformPool->SetBit<UPDATE_BIT>(entity);
                        }
                    }
                }
            });

        if (preUpdateTask) {
            simulateTask.succeed(preUpdateTask.value());
        }

        if (postUpdateTask) {
            postUpdateTask.value().succeed(simulateTask);
        }
    }
}