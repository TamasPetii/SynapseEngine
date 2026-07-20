#pragma once
#include "IApi.h"
#include "EditorCore/Types/EntityHandle.h"
#include <string>
#include <vector>
#include <cstdint>

namespace Syn {
    class IPipelineOverrideApi : public IApi {
    public:
        virtual ~IPipelineOverrideApi() = default;

        virtual bool HasPipelineOverride(EntityID entity) const = 0;

        virtual uint32_t GetExpectedSlotCount(EntityID entity) const = 0;

        virtual uint32_t GetPipelineAtSlot(EntityID entity, uint32_t slotIndex) const = 0;
        virtual void SetPipelineAtSlot(EntityID entity, uint32_t slotIndex, uint32_t pipelineType) = 0;

        virtual EntityID GetSharedPipelineEntity(EntityID entity) const = 0;
        virtual void SetSharedPipelineEntity(EntityID entity, EntityID sharedEntity) = 0;

        virtual std::vector<std::pair<EntityID, std::string>> GetCompatibleSharedEntities(EntityID entity) const = 0;
        virtual std::vector<std::pair<uint32_t, std::string>> GetAvailablePipelines() const = 0;
    };
}