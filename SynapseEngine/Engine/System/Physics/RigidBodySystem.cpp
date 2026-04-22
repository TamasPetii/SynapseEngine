#include "RigidBodySystem.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Physics/BoxColliderComponent.h"
#include "Engine/Component/Physics/SphereColliderComponent.h"
#include "Engine/Component/Physics/CapsuleColliderComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Physics/IPhysicsEngine.h"

#include "Engine/System/Physics/BoxColliderSystem.h"
#include "Engine/System/Physics/SphereColliderSystem.h"
#include "Engine/System/Physics/CapsuleColliderSystem.h"
#include "Engine/System/Core/TransformSystem.h"

namespace Syn
{
    std::vector<TypeID> RigidBodySystem::GetReadDependencies() const
    {
        return {
            TypeInfo<TransformSystem>::ID,
            TypeInfo<BoxColliderSystem>::ID,
            TypeInfo<SphereColliderSystem>::ID,
            TypeInfo<CapsuleColliderSystem>::ID
        };
    }

    std::vector<TypeID> RigidBodySystem::GetWriteDependencies() const
    {
        return { TypeInfo<RigidBodySystem>::ID };
    }

    void RigidBodySystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto rbPool = registry->GetPool<RigidBodyComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto boxPool = registry->GetPool<BoxColliderComponent>();
        auto spherePool = registry->GetPool<SphereColliderComponent>();
        auto capsulePool = registry->GetPool<CapsuleColliderComponent>();
        auto physicsEngine = ServiceLocator::GetPhysicsEngine();

        if (!rbPool || !physicsEngine) return;

        ParallelForEach(rbPool, subflow, SystemPhaseNames::Update, [rbPool, transformPool, boxPool, spherePool, capsulePool, physicsEngine](EntityID entity) {
            auto& rb = rbPool->Get(entity);

            bool hasBox = boxPool && boxPool->Has(entity);
            bool hasSphere = spherePool && spherePool->Has(entity);
            bool hasCapsule = capsulePool && capsulePool->Has(entity);

            if (!hasBox && !hasSphere && !hasCapsule)
            {
                if (rb.bodyID != INVALID_BODY_ID)
                {
                    physicsEngine->DestroyBody(rb.bodyID);
                    rb.bodyID = INVALID_BODY_ID;
                }
                return;
            }

            if (rb.bodyID == INVALID_BODY_ID && transformPool && transformPool->Has(entity))
            {
                auto& tr = transformPool->Get(entity);
                glm::quat rotQuat(glm::radians(tr.rotation));

                PhysicsBodySettings settings;
                settings.motionType = rb.motionType;
                settings.mass = rb.mass;
                settings.friction = rb.friction;
                settings.restitution = rb.restitution;
                settings.layer = rb.layer;

                if (hasBox) {
                    rb.bodyID = physicsEngine->CreateBoxBody(tr.translation, rotQuat, boxPool->Get(entity).halfExtents, settings);
                }
                else if (hasSphere) {
                    rb.bodyID = physicsEngine->CreateSphereBody(tr.translation, rotQuat, spherePool->Get(entity).radius, settings);
                }
                else if (hasCapsule) {
                    auto& cap = capsulePool->Get(entity);
                    rb.bodyID = physicsEngine->CreateCapsuleBody(tr.translation, rotQuat, cap.halfHeight, cap.radius, settings);
                }
            }
            else if (rb.bodyID != INVALID_BODY_ID && rbPool->IsBitSet<CHANGED_BIT>(entity))
            {
                physicsEngine->SetBodyFriction(rb.bodyID, rb.friction);
                physicsEngine->SetBodyRestitution(rb.bodyID, rb.restitution);
                physicsEngine->SetBodyMotionType(rb.bodyID, rb.motionType);
            }
            });
    }
}