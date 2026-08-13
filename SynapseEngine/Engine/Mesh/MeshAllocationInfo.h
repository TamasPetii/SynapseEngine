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
#include "Engine/SynApi.h"
#include "Engine/Material/MaterialRenderType.h"
#include "Engine/Render/PipelineRenderType.h"
#include <cstdint>
#include <vector>

namespace Syn
{
    struct alignas(16) SYN_API MeshAllocationInfo
    {
        uint32_t descriptorIndex;
        uint32_t padding[3];

        uint32_t indirectIndices[PipelineRenderType::PipelineRenderTypeCount][MaterialRenderType::MaterialRenderTypeCount];
        uint32_t instanceOffsets[PipelineRenderType::PipelineRenderTypeCount][MaterialRenderType::MaterialRenderTypeCount];
        uint32_t activeTypes[PipelineRenderType::PipelineRenderTypeCount][MaterialRenderType::MaterialRenderTypeCount];
    };

    struct alignas(16) SYN_API ModelAllocationInfo
    {
        uint32_t maxInstances;
        uint32_t meshAllocationOffset;
        uint32_t meshAllocationCount;
        uint32_t padding;
    };
}