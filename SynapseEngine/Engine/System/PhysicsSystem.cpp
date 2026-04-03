#include "PhysicsSystem.h"
#include "Engine/Component/TransformComponent.h"
#include "Engine/Component/RigidBodyComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Physics/IPhysicsEngine.h"

namespace Syn
{
    std::vector<TypeID> PhysicsSystem::GetReadDependencies() const {
        return { TypeInfo<RigidBodyComponent>::ID };
    }

    std::vector<TypeID> PhysicsSystem::GetWriteDependencies() const {
        return { TypeInfo<TransformComponent>::ID };
    }

    void PhysicsSystem::OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto rbPool = registry->GetPool<RigidBodyComponent>();

        IPhysicsEngine* physicsEngine = ServiceLocator::GetPhysicsEngine();

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
            physicsEngine->Update(deltaTime);
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