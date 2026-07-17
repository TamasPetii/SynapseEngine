#include "MaterialOverrideApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Rendering/MaterialOverrideComponent.h"
#include "Engine/Component/Rendering/ModelComponent.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Component/Core/TagComponent.h"

namespace Syn {
    bool MaterialOverrideApiImpl::HasMaterialOverride(EntityID entity) const {
        return EditorApiUtils::HasComponent<MaterialOverrideComponent>(_sceneManager, entity);
    }

    uint32_t MaterialOverrideApiImpl::GetExpectedSlotCount(EntityID entity) const {
        if (!EditorApiUtils::HasComponent<ModelComponent>(_sceneManager, entity)) return 0;

        uint32_t modelIdx = EditorApiUtils::ReadComponent<ModelComponent>(_sceneManager, entity, [](const auto& c) { return c.modelIndex; }, UINT32_MAX);
        if (modelIdx == UINT32_MAX) return 0;

        auto modelManager = ServiceLocator::Get<ModelManager>();
        if (!modelManager) return 0;

        auto snapshots = modelManager->GetResourceSnapshot();
        if (modelIdx < snapshots.size() && snapshots[modelIdx].state == ResourceState::Ready && snapshots[modelIdx].resource) {
            return static_cast<uint32_t>(snapshots[modelIdx].resource->cpuData.meshMaterialIndices.size());
        }
        return 0;
    }

    uint32_t MaterialOverrideApiImpl::GetMaterialAtSlot(EntityID entity, uint32_t slotIndex) const {
        return EditorApiUtils::ReadComponent<MaterialOverrideComponent>(_sceneManager, entity, [slotIndex](const auto& c) {
            if (slotIndex < c.materials.size()) {
                return c.materials[slotIndex];
            }
            return UINT32_MAX;
            }, UINT32_MAX);
    }

    void MaterialOverrideApiImpl::SetMaterialAtSlot(EntityID entity, uint32_t slotIndex, uint32_t materialId) {
        EditorApiUtils::ModifyComponent<MaterialOverrideComponent>(_sceneManager, entity, [&](auto& c, auto pool) {
            uint32_t expectedSize = GetExpectedSlotCount(entity);
            if (c.materials.size() != expectedSize) {
                c.materials.resize(expectedSize, UINT32_MAX);
            }
            if (slotIndex < c.materials.size()) {
                c.materials[slotIndex] = materialId;
                pool->template SetBit<CHANGED_BIT>(entity);
            }
            });
    }

    EntityID MaterialOverrideApiImpl::GetSharedMaterialEntity(EntityID entity) const {
        return EditorApiUtils::ReadComponent<MaterialOverrideComponent>(_sceneManager, entity, [](const auto& c) { return c.sharedMaterialEntity; }, NULL_ENTITY);
    }

    void MaterialOverrideApiImpl::SetSharedMaterialEntity(EntityID entity, EntityID sharedEntity) {
        EditorApiUtils::ModifyComponent<MaterialOverrideComponent>(_sceneManager, entity, [&](auto& c, auto pool) {
            c.sharedMaterialEntity = sharedEntity;
            pool->template SetBit<CHANGED_BIT>(entity);
            });
    }

    std::vector<std::pair<uint32_t, std::string>> MaterialOverrideApiImpl::GetAvailableMaterials() const {
        std::vector<std::pair<uint32_t, std::string>> result;

        auto matManager = ServiceLocator::Get<MaterialManager>();
        if (!matManager) return result;

        auto paths = matManager->GetResourcePaths();
        auto snapshots = matManager->GetResourceSnapshot();

        for (uint32_t i = 0; i < paths.size(); ++i) {
            if (snapshots[i].state == ResourceState::Ready) {
                result.push_back({ i, paths[i] });
            }
        }

        return result;
    }

    std::vector<std::pair<EntityID, std::string>> MaterialOverrideApiImpl::GetCompatibleSharedEntities(EntityID targetEntity) const {
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