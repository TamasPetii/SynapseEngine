// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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