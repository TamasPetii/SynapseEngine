#include "BoxColliderSystem.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/System/Core/TransformSystem.h"
#include "Engine/Component/Physics/RigidBodyComponent.h"
#include "Engine/System/Physics/RigidBodySystem.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Physics/IPhysicsEngine.h"
#include "PhysicsUtils.h"

namespace Syn
{
    std::vector<TypeID> BoxColliderSystem::GetReadDependencies() const
    {
        return { TypeInfo<TransformSystem>::ID };
    }

    std::vector<TypeID> BoxColliderSystem::GetWriteDependencies() const
    {
        return { TypeInfo<RigidBodySystem>::ID, TypeInfo<BoxColliderSystem>::ID };
    }

    void BoxColliderSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto boxPool = registry->GetPool<BoxColliderComponent>();
        auto rbPool = registry->GetPool<RigidBodyComponent>();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto physicsEngine = ServiceLocator::GetPhysicsEngine();

        if (!boxPool || !rbPool || !transformPool || !physicsEngine) return;

        ParallelForEach(boxPool, subflow, SystemPhaseNames::Update, [boxPool, rbPool, transformPool, physicsEngine](EntityID entity) {

            if (!rbPool->Has(entity) || !transformPool->Has(entity)) return;

            auto& box = boxPool->Get(entity);
            auto& rb = rbPool->Get(entity);
            auto& tr = transformPool->Get(entity);

            const glm::mat4& mat = tr.transform;

            glm::vec3 worldScale(
                glm::length(glm::vec3(mat[0])),
                glm::length(glm::vec3(mat[1])),
                glm::length(glm::vec3(mat[2]))
            );

            if (rb.bodyID == INVALID_BODY_ID)
            {
                rb.bodyID = PhysicsUtils::TryCreateBody(entity, &tr, rb,
                    [&](const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale, const PhysicsBodySettings& settings) {
                        return physicsEngine->CreateBoxBody(pos, rot, box.halfExtents * scale, settings);
                    });
            }
            else if (boxPool->IsBitSet<UPDATE_BIT>(entity))
            {
                glm::vec3 worldScale = PhysicsUtils::ExtractScale(tr.transform);
                physicsEngine->SetBoxShape(rb.bodyID, box.halfExtents * worldScale);
            }

            box.version++;
            });
    }

    void BoxColliderSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto boxPool = registry->GetPool<BoxColliderComponent>();
        if (!boxPool) return;

        auto componentBuffer = componentBufferManager->GetComponentBuffer(BufferNames::BoxColliderData, frameIndex);
        if (!componentBuffer.buffer) return;

        auto bufferHandler = static_cast<BoxColliderComponentGPU*>(componentBuffer.buffer->Map());

        auto processUpload = [boxPool, bufferHandler, componentBuffer](EntityID entity) {
            auto& box = boxPool->Get(entity);
            auto boxIndex = boxPool->GetMapping().Get(entity);

            if (componentBuffer.versions[boxIndex] != box.version)
            {
                componentBuffer.versions[boxIndex] = box.version;
                bufferHandler[boxIndex] = BoxColliderComponentGPU(box);
            }
            };

        ForEachStream(boxPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadDynamic)
            ForEachDynamic(boxPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadStatic)
            ForEachStatic(boxPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}