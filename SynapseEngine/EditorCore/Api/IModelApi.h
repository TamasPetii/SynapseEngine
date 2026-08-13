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
#include <cstdint>
#include <vector>
#include <string>
#include "IApi.h"
#include "Engine/Mesh/Data/Cpu/CpuModelData.h"

namespace Syn
{
    constexpr uint32_t INVALID_MODEL_ID = 0xFFFFFFFF;
    constexpr int32_t INVALID_NODE_INDEX = -1;

    struct ModelItemData {
        uint32_t id;
        std::string name;
        std::string path;
    };

    class IModelApi : public IApi {
    public:
        virtual ~IModelApi() = default;

        virtual std::vector<ModelItemData> GetAllModels() const = 0;
        virtual uint64_t GetVersion() const = 0;

        virtual void SetSelected(uint32_t modelId, int32_t nodeIndex) = 0;
        virtual std::pair<uint32_t, int32_t> GetSelected() const = 0;

        virtual const CpuModelData* GetModelCpuData(uint32_t modelId) const = 0;
        virtual std::string GetNodeName(uint32_t modelId, uint16_t nodeIndex) const = 0;

        virtual void ApplyModelToPreviewObject(uint32_t modelId) = 0;
    };
}