#pragma once
#include "EditorCore/Api/IMaterialOverrideApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class MaterialOverrideApiImpl : public IMaterialOverrideApi {
    public:
        MaterialOverrideApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasMaterialOverride(EntityID entity) const override;

        uint32_t GetExpectedSlotCount(EntityID entity) const override;
        uint32_t GetMaterialAtSlot(EntityID entity, uint32_t slotIndex) const override;
        void SetMaterialAtSlot(EntityID entity, uint32_t slotIndex, uint32_t materialId) override;

        EntityID GetSharedMaterialEntity(EntityID entity) const override;
        void SetSharedMaterialEntity(EntityID entity, EntityID sharedEntity) override;

        std::vector<std::pair<EntityID, std::string>> GetCompatibleSharedEntities(EntityID entity) const override;
        std::vector<std::pair<uint32_t, std::string>> GetAvailableMaterials() const override;
    private:
        SceneManager* _sceneManager;
    };
}