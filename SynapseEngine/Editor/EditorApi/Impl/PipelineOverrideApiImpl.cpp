#include "PipelineOverrideApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Rendering/PipelineOverrideComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Component/Core/TagComponent.h"

namespace Syn {
    bool PipelineOverrideApiImpl::HasPipelineOverride(EntityID entity) const {
        return EditorApiUtils::HasComponent<PipelineOverrideComponent>(_sceneManager, entity);
    }

    uint32_t PipelineOverrideApiImpl::GetExpectedSlotCount(EntityID entity) const {
        if (!EditorApiUtils::HasComponent<ModelComponent>(_sceneManager, entity)) return 0;

        uint32_t modelIdx = EditorApiUtils::ReadComponent<ModelComponent>(_sceneManager, entity, [](const auto& c) { return c.modelIndex; }, UINT32_MAX);
        if (modelIdx == UINT32_MAX) return 0;

        auto modelManager = ServiceLocator::Get<ModelManager>();
        if (!modelManager) return 0;

        auto snapshots = modelManager->GetResourceSnapshot();
        if (modelIdx < snapshots.size() && snapshots[modelIdx].state == ResourceState::Ready && snapshots[modelIdx].resource) {
            return snapshots[modelIdx].resource->cpuData.globalMeshCount;
        }
        return 0;
    }

    uint32_t PipelineOverrideApiImpl::GetPipelineAtSlot(EntityID entity, uint32_t slotIndex) const {
        return EditorApiUtils::ReadComponent<PipelineOverrideComponent>(_sceneManager, entity, [slotIndex](const auto& c) {
            if (slotIndex < c.pipelines.size()) {
                return c.pipelines[slotIndex];
            }
            return UINT32_MAX;
            }, UINT32_MAX);
    }

    void PipelineOverrideApiImpl::SetPipelineAtSlot(EntityID entity, uint32_t slotIndex, uint32_t pipelineType) {
        EditorApiUtils::ModifyComponent<PipelineOverrideComponent>(_sceneManager, entity, [&](auto& c, auto pool) {
            uint32_t expectedSize = GetExpectedSlotCount(entity);
            if (c.pipelines.size() != expectedSize) {
                c.pipelines.resize(expectedSize, UINT32_MAX);
            }
            if (slotIndex < c.pipelines.size()) {
                c.pipelines[slotIndex] = pipelineType;
                pool->template SetBit<CHANGED_BIT>(entity);
            }
            });
    }

    EntityID PipelineOverrideApiImpl::GetSharedPipelineEntity(EntityID entity) const {
        return EditorApiUtils::ReadComponent<PipelineOverrideComponent>(_sceneManager, entity, [](const auto& c) { return c.sharedPipelineEntity; }, NULL_ENTITY);
    }

    void PipelineOverrideApiImpl::SetSharedPipelineEntity(EntityID entity, EntityID sharedEntity) {
        EditorApiUtils::ModifyComponent<PipelineOverrideComponent>(_sceneManager, entity, [&](auto& c, auto pool) {
            c.sharedPipelineEntity = sharedEntity;
            pool->template SetBit<CHANGED_BIT>(entity);
            });
    }

    std::vector<std::pair<uint32_t, std::string>> PipelineOverrideApiImpl::GetAvailablePipelines() const {
        return {
            { 0, "Traditional" },
            { 1, "Meshlet" }
        };
    }

    std::vector<std::pair<EntityID, std::string>> PipelineOverrideApiImpl::GetCompatibleSharedEntities(EntityID targetEntity) const {
        std::vector<std::pair<EntityID, std::string>> result;

        auto scene = _sceneManager->GetActiveScene();
        if (!scene || !scene->GetRegistry()) return result;

        uint32_t targetSlotCount = GetExpectedSlotCount(targetEntity);
        if (targetSlotCount == 0) return result;

        auto registry = scene->GetRegistry();

        for (EntityID entity : registry->GetActiveEntities().GetDenseEntities()) {
            if (entity == targetEntity) continue;

            if (registry->HasComponent<ModelComponent>(entity)) {
                uint32_t currentSlotCount = GetExpectedSlotCount(entity);

                if (currentSlotCount == targetSlotCount) {
                    std::string entityName = "Entity " + std::to_string(entity);
                    if (registry->HasComponent<TagComponent>(entity)) {
                        entityName = registry->GetComponent<TagComponent>(entity).name;
                    }

                    result.push_back({ entity, entityName + " (ID: " + std::to_string(entity) + ")" });
                }
            }
        }

        return result;
    }
}