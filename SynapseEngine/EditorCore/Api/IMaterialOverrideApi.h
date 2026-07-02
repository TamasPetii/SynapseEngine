#pragma once
#include "EditorCore/Types/EntityHandle.h"
#include <string>
#include <vector>
#include <cstdint>

namespace Syn {
    class IMaterialOverrideApi {
    public:
        virtual ~IMaterialOverrideApi() = default;

        virtual bool HasMaterialOverride(EntityID entity) const = 0;

        virtual uint32_t GetExpectedSlotCount(EntityID entity) const = 0;

        virtual uint32_t GetMaterialAtSlot(EntityID entity, uint32_t slotIndex) const = 0;
        virtual void SetMaterialAtSlot(EntityID entity, uint32_t slotIndex, uint32_t materialId) = 0;

        virtual EntityID GetSharedMaterialEntity(EntityID entity) const = 0;
        virtual void SetSharedMaterialEntity(EntityID entity, EntityID sharedEntity) = 0;

        virtual std::vector<std::pair<EntityID, std::string>> GetCompatibleSharedEntities(EntityID entity) const = 0;
        virtual std::vector<std::pair<uint32_t, std::string>> GetAvailableMaterials() const = 0;
    };
}