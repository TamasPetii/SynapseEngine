#pragma once
#include "EditorCore/Api/IPipelineOverrideApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class PipelineOverrideApiImpl : public IPipelineOverrideApi {
    public:
        PipelineOverrideApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasPipelineOverride(EntityID entity) const override;

        uint32_t GetExpectedSlotCount(EntityID entity) const override;
        uint32_t GetPipelineAtSlot(EntityID entity, uint32_t slotIndex) const override;
        void SetPipelineAtSlot(EntityID entity, uint32_t slotIndex, uint32_t pipelineType) override;

        EntityID GetSharedPipelineEntity(EntityID entity) const override;
        void SetSharedPipelineEntity(EntityID entity, EntityID sharedEntity) override;

        std::vector<std::pair<EntityID, std::string>> GetCompatibleSharedEntities(EntityID entity) const override;
        std::vector<std::pair<uint32_t, std::string>> GetAvailablePipelines() const override;
    private:
        SceneManager* _sceneManager;
    };
}