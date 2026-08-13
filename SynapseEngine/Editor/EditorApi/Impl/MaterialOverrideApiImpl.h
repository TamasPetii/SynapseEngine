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