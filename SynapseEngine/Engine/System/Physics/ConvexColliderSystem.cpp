#include "ConvexColliderSystem.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/System/Core/TransformSystem.h"
#include "Engine/Component/Physics/RigidBodyComponent.h"
#include "Engine/System/Physics/RigidBodySystem.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Physics/IPhysicsEngine.h"
#include "PhysicsUtils.h"
#include "Engine/System/Rendering/ModelSystem.h"

namespace Syn
{
    std::vector<TypeID> ConvexColliderSystem::GetReadDependencies() const
    {
        return { 
            TypeInfo<TransformSystem>::ID,
             TypeInfo<ModelSystem>::ID,
        };
    }

    std::vector<TypeID> ConvexColliderSystem::GetWriteDependencies() const
    {
        return { 
            TypeInfo<RigidBodySystem>::ID,
            TypeInfo<ConvexColliderSystem>::ID
        };
    }

    void ConvexColliderSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto convexPool = registry->GetPool<ConvexColliderComponent>();
        auto rbPool = registry->GetPool<RigidBodyComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto modelPool = registry->GetPool<ModelComponent>();
        auto physicsEngine = scene->GetPhysicsEngine();

        if (!convexPool || !rbPool || !transformPool || !modelPool || !physicsEngine) 
            return;

        auto& modelSnapshots = scene->GetSystemContext().modelSnapshots;

        ParallelForEach(convexPool, subflow, SystemPhaseNames::Update, [convexPool, rbPool, transformPool, modelPool, physicsEngine, modelSnapshots](EntityID entity) {

            if (!rbPool->Has(entity) || !transformPool->Has(entity) || !modelPool->Has(entity)) 
                return;

            auto& convex = convexPool->Get(entity);
            auto& rb = rbPool->Get(entity);
            auto& tr = transformPool->Get(entity);
            auto& modelComp = modelPool->Get(entity);

            uint32_t modelIndex = modelComp.modelIndex;
            if (modelIndex >= modelSnapshots.size()) return;

            auto model = modelSnapshots[modelIndex].resource;
            if (!model || model->cpuData.vertices.empty()) return;

            const auto& vertices = model->cpuData.physicsVertices;

            if (rb.bodyID == INVALID_BODY_ID)
            {
                rb.bodyID = PhysicsUtils::TryCreateBody(entity, &tr, rb,
                    [&](const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale, const PhysicsBodySettings& settings) {
                        return physicsEngine->CreateConvexBody(pos, rot, vertices, scale, settings);
                    });
            }
            else if (convexPool->IsBitSet<UPDATE_BIT>(entity))
            {
                glm::vec3 worldScale = PhysicsUtils::ExtractScale(tr.transform);
                physicsEngine->SetConvexShape(rb.bodyID, vertices, worldScale);
            }

            convex.version++;
            });
    }

    void ConvexColliderSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto convexPool = registry->GetPool<ConvexColliderComponent>();
        if (!convexPool) return;

        auto componentBuffer = componentBufferManager->GetComponentBuffer(BufferNames::ConvexColliderData, frameIndex);
        if (!componentBuffer.buffer) return;

        auto bufferHandler = static_cast<ConvexColliderComponentGPU*>(componentBuffer.buffer->Map());

        auto processUpload = [convexPool, bufferHandler, componentBuffer](EntityID entity) {
            auto& convex = convexPool->Get(entity);
            auto index = convexPool->GetMapping().Get(entity);

            if (componentBuffer.versions[index] != convex.version)
            {
                componentBuffer.versions[index] = convex.version;
                bufferHandler[index] = ConvexColliderComponentGPU(convex);
            }
            };

        ForEachStream(convexPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadDynamic) ForEachDynamic(convexPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
        if (uploadStatic) ForEachStatic(convexPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}