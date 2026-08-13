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
#include "EditorCore/Api/IMaterialApi.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    enum class GraphPinType;

    class MaterialApiImpl : public IMaterialApi {
    public:
        MaterialApiImpl(MaterialManager* materialManager, SceneManager* sceneManager)
            : _materialManager(materialManager), _sceneManager(sceneManager) {}

        std::vector<MaterialItemData> GetAllMaterials() const override;
        uint32_t GetSelectedMaterial() const override;
        void SetSelectedMaterial(uint32_t id) override;
        uint64_t GetVersion() const override;

        std::string GetMaterialName(uint32_t materialId) const override;
        uint32_t GetLinkedTexture(uint32_t materialId, uint32_t textureType) const override;

        void LinkTextureToMaterial(uint32_t materialId, uint32_t textureType, uint32_t textureId) override;
        void UnlinkTextureFromMaterial(uint32_t materialId, uint32_t textureType) override;

        bool GetMaterialData(uint32_t materialId, Material& outMaterial) const override;
        void UpdateMaterialData(uint32_t materialId, const Material& material) override;

        void ApplyMaterialToPreviewObjects(uint32_t materialId) override;
    private:
        SceneManager* _sceneManager;
        MaterialManager* _materialManager;
        uint32_t _selectedMaterial = INVALID_MATERIAL_ID;
    };
}