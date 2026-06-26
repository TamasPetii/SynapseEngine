#include "RigidBodySystem.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Component/Physics/BoxColliderComponent.h"
#include "Engine/Component/Physics/SphereColliderComponent.h"
#include "Engine/Component/Physics/CapsuleColliderComponent.h"
#include "Engine/Component/Physics/ConvexColliderComponent.h"
#include "Engine/Component/Physics/MeshColliderComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/System/Physics/PhysicsUtils.h"

#include "Engine/ServiceLocator.h"
#include "Engine/Physics/IPhysicsEngine.h"

#include "Engine/System/Physics/BoxColliderSystem.h"
#include "Engine/System/Physics/SphereColliderSystem.h"
#include "Engine/System/Physics/CapsuleColliderSystem.h"
#include "Engine/System/Physics/ConvexColliderSystem.h"
#include "Engine/System/Physics/MeshColliderSystem.h"
#include "Engine/System/Rendering/ModelSystem.h"
#include "Engine/System/Core/TransformSystem.h"

#include "Engine/Mesh/ModelManager.h"

namespace Syn
{
    std::vector<TypeID> RigidBodySystem::GetReadDependencies() const
    {
        return {
            TypeInfo<TransformSystem>::ID,
            TypeInfo<BoxColliderSystem>::ID,
            TypeInfo<SphereColliderSystem>::ID,
            TypeInfo<CapsuleColliderSystem>::ID,
            TypeInfo<CapsuleColliderSystem>::ID,
            TypeInfo<ConvexColliderSystem>::ID,
            TypeInfo<MeshColliderSystem>::ID,
            TypeInfo<ModelSystem>::ID
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
        auto convexPool = registry->GetPool<ConvexColliderComponent>();
        auto meshPool = registry->GetPool<MeshColliderComponent>();
        auto modelPool = registry->GetPool<ModelComponent>();

        auto physicsEngine = scene->GetPhysicsEngine();
        auto modelManager = ServiceLocator::GetModelManager();

        if (!rbPool || !physicsEngine || !modelManager) return;
        auto modelSnapshots = modelManager->GetResourceSnapshot();

        ParallelForEach(rbPool, subflow, SystemPhaseNames::Update, [rbPool, transformPool, boxPool, spherePool, capsulePool, physicsEngine, convexPool, meshPool, modelPool, modelSnapshots](EntityID entity) {
            auto& rb = rbPool->Get(entity);

            bool hasBox = boxPool && boxPool->Has(entity);
            bool hasSphere = spherePool && spherePool->Has(entity);
            bool hasCapsule = capsulePool && capsulePool->Has(entity);
            bool hasConvex = convexPool && convexPool->Has(entity);
            bool hasMesh = meshPool && meshPool->Has(entity);

            if (!hasBox && !hasSphere && !hasCapsule && !hasConvex && !hasMesh)
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
                glm::vec3 scale = PhysicsUtils::ExtractScale(tr.transform);

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
                else if (hasConvex) {
                    if (modelPool && modelPool->Has(entity)) {
                        auto& modelComp = modelPool->Get(entity);
                        if (modelComp.modelIndex < modelSnapshots.size()) {
                            auto model = modelSnapshots[modelComp.modelIndex].resource;
                            if (model && !model->cpuData.physicsVertices.empty()) {
                                rb.bodyID = physicsEngine->CreateConvexBody(tr.translation, rotQuat, model->cpuData.physicsVertices, scale, settings);
                            }
                        }
                    }
                }
                else if (hasMesh) {
                    if (modelPool && modelPool->Has(entity)) {
                        auto& modelComp = modelPool->Get(entity);
                        auto& meshColl = meshPool->Get(entity);

                        if (modelComp.modelIndex < modelSnapshots.size()) {
                            auto model = modelSnapshots[modelComp.modelIndex].resource;
                            uint32_t lod = glm::min(meshColl.targetLodLevel, 3u);

                            if (model && !model->cpuData.physicsVertices.empty() &&
                                lod < model->cpuData.physicsIndicesPerLod.size() &&
                                !model->cpuData.physicsIndicesPerLod[lod].empty())
                            {
                                rb.bodyID = physicsEngine->CreateMeshBody(
                                    tr.translation,
                                    rotQuat,
                                    model->cpuData.physicsVertices,
                                    model->cpuData.physicsIndicesPerLod[lod],
                                    scale,
                                    settings
                                );
                            }
                        }
                    }
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