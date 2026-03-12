#include "TransformSystem.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Syn
{
    std::vector<TypeID> TransformSystem::GetWriteDependencies() const
    {
        { return { TypeInfo<TransformSystem>::ID }; }
    }

    void TransformSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto transformPool = registry->GetPool<TransformComponent>();
        if (!transformPool) return;

        ParallelForEachIf<UPDATE_BIT>(transformPool, subflow, [transformPool](EntityID entity) {
            auto& transformComponent = transformPool->Get(entity);
            DenseIndex transformIndex = transformPool->GetMapping().Get(entity);

            transformComponent.transform = glm::mat4(1.0f);
            transformComponent.transform = glm::translate(transformComponent.transform, transformComponent.translation);
            transformComponent.transform = glm::rotate(transformComponent.transform, glm::radians(transformComponent.rotation.z), glm::vec3(0, 0, 1));
            transformComponent.transform = glm::rotate(transformComponent.transform, glm::radians(transformComponent.rotation.y), glm::vec3(0, 1, 0));
            transformComponent.transform = glm::rotate(transformComponent.transform, glm::radians(transformComponent.rotation.x), glm::vec3(1, 0, 0));
            transformComponent.transform = glm::scale(transformComponent.transform, transformComponent.scale);
            transformComponent.transformIT = glm::transpose(glm::inverse(transformComponent.transform));

            if (transformPool->GetStorage().IsDynamic(transformIndex))
                transformPool->SetBit<CHANGED_BIT>(entity);

            transformComponent.version++;
            });
    }

    void TransformSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto transformPool = registry->GetPool<TransformComponent>();
        if (!transformPool) return;

        auto componentBuffer = componentBufferManager->GetComponentBuffer(BufferNames::TransformData, frameIndex);
        if (!componentBuffer.buffer) return;

        auto bufferHandler = static_cast<TransformComponentGPU*>(componentBuffer.buffer->Map());

        auto processUpload = [transformPool, bufferHandler, componentBuffer](EntityID entity) {
            auto& transformComponent = transformPool->Get(entity);
            auto transformIndex = transformPool->GetMapping().Get(entity);

            if (componentBuffer.versions[transformIndex] != transformComponent.version)
            {
                componentBuffer.versions[transformIndex] = transformComponent.version;
                bufferHandler[transformIndex] = TransformComponentGPU(transformComponent);
            }
            };

        ForEachStream(transformPool, subflow, processUpload);

        if (uploadDynamic)
        {
            ForEachDynamic(transformPool, subflow, processUpload);
            
            //Full range needed! UpdateDynamic problematic?
            ForEachStatic(transformPool, subflow, processUpload);
        }
    }
}