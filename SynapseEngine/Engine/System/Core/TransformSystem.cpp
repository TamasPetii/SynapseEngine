#include "TransformSystem.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <print>
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/System/Rendering/ModelSystem.h"
#include "TransformSetupSystem.h"

namespace Syn
{
    std::vector<TypeID> TransformSystem::GetReadDependencies() const
    {
        return { TypeInfo<TransformSetupSystem>::ID };
    }

    std::vector<TypeID> TransformSystem::GetWriteDependencies() const
    {
        return { TypeInfo<TransformSystem>::ID };
    }

    void TransformSystem::UpdateComponents(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow)
    {
        auto registry = scene->GetRegistry();
        auto transformPool = registry->GetPool<TransformComponent>();
        auto hierarchyPool = registry->GetPool<HierarchyComponent>();
        auto hierarchyManager = scene->GetHierarchyManager();

        if (!transformPool || !hierarchyPool || !hierarchyManager) return;

        auto* workQueue = hierarchyManager->EnsureWorkQueue<TransformComponent>();
        uint32_t maxLevel = hierarchyManager->GetMaxActiveLevel();
        std::vector<tf::Task> levelTasks;

        for (uint32_t level = 0; level < maxLevel; ++level)
        {
            tf::Task levelTask = this->EmplaceTask(subflow, "TransformMath_" + std::to_string(level), [=](tf::Subflow& nested_subflow) {

                auto currentQueue = workQueue->GetQueue(level);

                if (currentQueue.empty()) return;

                nested_subflow.for_each(currentQueue.begin(), currentQueue.end(), [=](EntityID entity) {

                    auto& transformComponent = transformPool->Get(entity);

                    glm::mat4 localMat = glm::translate(glm::mat4(1.0f), transformComponent.translation);
                    localMat = glm::rotate(localMat, glm::radians(transformComponent.rotation.z), glm::vec3(0, 0, 1));
                    localMat = glm::rotate(localMat, glm::radians(transformComponent.rotation.y), glm::vec3(0, 1, 0));
                    localMat = glm::rotate(localMat, glm::radians(transformComponent.rotation.x), glm::vec3(1, 0, 0));
                    localMat = glm::scale(localMat, transformComponent.scale);

                    if (hierarchyPool->Has(entity))
                    {
                        EntityID parent = hierarchyPool->Get(entity).parent;

                        if (parent != NULL_ENTITY && transformPool->Has(parent)) {
                            auto& parentTransform = transformPool->Get(parent);
                            transformComponent.transform = parentTransform.transform * localMat;
                        }
                        else {
                            transformComponent.transform = localMat;
                        }
                    }
                    else
                    {
                        transformComponent.transform = localMat;
                    }

                    transformComponent.transformIT = glm::transpose(glm::inverse(transformComponent.transform));

                    if (transformPool->IsDynamic(entity)) {
                        transformPool->SetBit<CHANGED_BIT>(entity);
                    }

                    transformComponent.version++;
                    });
                });

            if (!levelTasks.empty()) {
                levelTasks.back().precede(levelTask);
            }
            levelTasks.push_back(levelTask);
        }
    }

    void TransformSystem::UploadComponents(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow, bool uploadDynamic, bool uploadStatic)
    {
        auto registry = scene->GetRegistry();
        auto componentBufferManager = scene->GetComponentBufferManager();
        auto transformPool = registry->GetPool<TransformComponent>();
        if (!transformPool) return;

        auto transformDataBuffer = componentBufferManager->GetComponentBuffer(BufferNames::TransformData, frameIndex);
        if (!transformDataBuffer.buffer) return;
        auto transformDataBufferHandler = static_cast<TransformComponentGPU*>(transformDataBuffer.buffer->Map());

		auto transformLinkBuffer = componentBufferManager->GetComponentBuffer(BufferNames::TransformModelLinkData, frameIndex);
        if(!transformLinkBuffer.buffer) return;
		auto transformLinkBufferHandler = static_cast<TransformModelLinkGPU*>(transformLinkBuffer.buffer->Map());

        auto processUpload = [transformPool, transformDataBuffer, transformDataBufferHandler, transformLinkBuffer, transformLinkBufferHandler](EntityID entity) {
            auto& transformComponent = transformPool->Get(entity);
            auto transformIndex = transformPool->GetMapping().Get(entity);

            if (transformDataBuffer.versions[transformIndex] != transformComponent.version)
            {
                transformDataBuffer.versions[transformIndex] = transformComponent.version;
                transformDataBufferHandler[transformIndex] = TransformComponentGPU(transformComponent);
            }

            if (transformLinkBuffer.versions[transformIndex] != transformComponent.version)
            {
                transformLinkBuffer.versions[transformIndex] = transformComponent.version;
                transformLinkBufferHandler[transformIndex].entityIndex = entity;
            }
            };

        ForEachStream(transformPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadDynamic)
            ForEachDynamic(transformPool, subflow, SystemPhaseNames::UploadGPU, processUpload);

        if (uploadStatic)
            ForEachStatic(transformPool, subflow, SystemPhaseNames::UploadGPU, processUpload);
    }
}