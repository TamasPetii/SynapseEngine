#include "TransformSystem.h"
#include "Engine/Component/TransformComponent.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Syn
{
    std::vector<TypeID> TransformSystem::GetReadDependencies() const
    {
        return {};
    }

    std::vector<TypeID> TransformSystem::GetWriteDependencies() const
    {
        return { TypeInfo<TransformComponent>::ID };
    }

    std::string TransformSystem::GetName() const
    {
        return "TransformSystem";
    }

    void TransformSystem::OnUpdate(std::shared_ptr<Registry> registry, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto pool = registry->GetPool<TransformComponent>();
        if (!pool) return;

        auto processEntity = [&pool](EntityID entity)  {
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
        };

        auto streamSpan = pool->GetStorage().GetStreamEntities();
        if (!streamSpan.empty())
        {
            subflow.for_each(streamSpan.begin(), streamSpan.end(), processEntity);
        }

        auto dynamicSpan = pool->GetStorage().GetDynamicEntities();
        if (!dynamicSpan.empty())
        {
            subflow.for_each(dynamicSpan.begin(), dynamicSpan.end(), [&pool, &processEntity](EntityID entity) {
                if (pool->IsBitSet<UPDATE_BIT>(entity))
                {
                    processEntity(entity);
                }
                });
        }

        auto staticDirtySpan = pool->GetStorage().GetDirtyStatics();
        if (!staticDirtySpan.empty())
        {
            subflow.for_each(staticDirtySpan.begin(), staticDirtySpan.end(), processEntity);
        }
    }

    void TransformSystem::OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ComponentBufferManager> componentBufferManager, uint32_t frameIndex, tf::Subflow& subflow)
    {
        auto pool = registry->GetPool<TransformComponent>();
        if (!pool) return;

        auto componentBuffer = componentBufferManager->GetComponentBuffer("TransformData", frameIndex);
        if (!componentBuffer.buffer) return;

        auto bufferHandler = static_cast<TransformComponentGPU*>(componentBuffer.buffer->Map());

        auto processUpload = [&pool, bufferHandler, componentBuffer](EntityID entity) {
            auto& transformComponent = pool->Get(entity);
            auto transformIndex = pool->GetMapping().Get(entity);

            if (componentBuffer.versions[transformIndex] != transformComponent.version)
            {
                componentBuffer.versions[transformIndex] = transformComponent.version;
                bufferHandler[transformIndex] = TransformComponentGPU(transformComponent);
            }
            };

        auto streamSpan = pool->GetStorage().GetStreamEntities();
        if (!streamSpan.empty())
        {
            subflow.for_each(streamSpan.begin(), streamSpan.end(), processUpload);
        }

        auto dynamicSpan = pool->GetStorage().GetDynamicEntities();
        if (!dynamicSpan.empty())
        {
            subflow.for_each(dynamicSpan.begin(), dynamicSpan.end(), processUpload);
        }

        auto staticDirtySpan = pool->GetStorage().GetDirtyStatics();
        if (!staticDirtySpan.empty())
        {
            subflow.for_each(staticDirtySpan.begin(), staticDirtySpan.end(), processUpload);
        }
    }

    void TransformSystem::OnFinish(std::shared_ptr<Registry> registry, tf::Subflow& subflow)
    {
        auto pool = registry->GetPool<TransformComponent>();
        if (!pool) return;

        auto processFinish = [&pool](EntityID entity) {
            if (pool->IsBitSet<CHANGED_BIT>(entity))
            {
                pool->ResetBit<CHANGED_BIT>(entity);
            }
            if (pool->IsBitSet<UPDATE_BIT>(entity))
            {
                pool->ResetBit<UPDATE_BIT>(entity);
            }
            };

        auto dynamicSpan = pool->GetStorage().GetDynamicEntities();
        if (!dynamicSpan.empty())
        {
            subflow.for_each(dynamicSpan.begin(), dynamicSpan.end(), processFinish);
        }

        //Todo: Reset all dirty static list!!!
    }
}