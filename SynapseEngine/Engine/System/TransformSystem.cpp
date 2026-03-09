#include "TransformSystem.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Syn
{
    void TransformSystem::OnUpdate(std::shared_ptr<Registry> registry, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto pool = registry->GetPool<TransformComponent>();
        if (!pool) return;

        ParallelForEach(pool, subflow, [&pool](EntityID entity) {
            auto& transformComponent = pool->Get(entity);

            transformComponent.transform = glm::mat4(1.0f);
            transformComponent.transform = glm::translate(transformComponent.transform, transformComponent.translation);
            transformComponent.transform = glm::rotate(transformComponent.transform, glm::radians(transformComponent.rotation.z), glm::vec3(0, 0, 1));
            transformComponent.transform = glm::rotate(transformComponent.transform, glm::radians(transformComponent.rotation.y), glm::vec3(0, 1, 0));
            transformComponent.transform = glm::rotate(transformComponent.transform, glm::radians(transformComponent.rotation.x), glm::vec3(1, 0, 0));
            transformComponent.transform = glm::scale(transformComponent.transform, transformComponent.scale);
            transformComponent.transformIT = glm::transpose(glm::inverse(transformComponent.transform));

            pool->SetBit<CHANGED_BIT>(entity);
            transformComponent.version++;
            });
    }

    void TransformSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ComponentBufferManager> componentBufferManager, uint32_t frameIndex, tf::Subflow& subflow)
    {
        auto pool = registry->GetPool<TransformComponent>();
        if (!pool) return;

        auto componentBuffer = componentBufferManager->GetComponentBuffer("TransformData", frameIndex);
        if (!componentBuffer.buffer) return;

        auto bufferHandler = static_cast<TransformComponentGPU*>(componentBuffer.buffer->Map());

        ParallelForEach(pool, subflow, [&pool, bufferHandler, componentBuffer](EntityID entity) {
            auto& transformComponent = pool->Get(entity);
            auto transformIndex = pool->GetMapping().Get(entity);

            if (componentBuffer.versions[transformIndex] != transformComponent.version)
            {
                componentBuffer.versions[transformIndex] = transformComponent.version;
                bufferHandler[transformIndex] = TransformComponentGPU(transformComponent);
            }
            });
    }
}