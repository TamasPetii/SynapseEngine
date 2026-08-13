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
#include "IApi.h"
#include <cstdint>
#include <vector>
#include <string>
#include "Engine/Material/Material.h"
//Todo: Domain Material!!

namespace Syn 
{
    constexpr uint32_t INVALID_MATERIAL_ID = 0xFFFFFFFF;

    struct MaterialItemData {
        uint32_t id;
        std::string name;
        std::string path;
    };

    class IMaterialApi : public IApi {
    public:
        virtual ~IMaterialApi() = default;

        virtual std::vector<MaterialItemData> GetAllMaterials() const = 0;
        virtual uint32_t GetSelectedMaterial() const = 0;
        virtual void SetSelectedMaterial(uint32_t id) = 0;
        virtual uint64_t GetVersion() const = 0;

        virtual std::string GetMaterialName(uint32_t materialId) const = 0;
        virtual uint32_t GetLinkedTexture(uint32_t materialId, uint32_t textureType) const = 0;
        virtual void LinkTextureToMaterial(uint32_t materialId, uint32_t textureType, uint32_t textureId) = 0;
        virtual void UnlinkTextureFromMaterial(uint32_t materialId, uint32_t textureType) = 0;

        virtual bool GetMaterialData(uint32_t materialId, Material& outMaterial) const = 0;
        virtual void UpdateMaterialData(uint32_t materialId, const Material& material) = 0;

        virtual void ApplyMaterialToPreviewObjects(uint32_t materialId) = 0;
    };
}