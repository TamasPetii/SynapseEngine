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
#include <cstdint>

namespace Syn
{
    struct SYN_API MeshDrawDescriptor
    {
        uint32_t modelIndex;        // Model ID for vertex/index/material buffer lookups
        uint32_t meshIndex;         // Submesh index
        uint32_t lodIndex;          // Level of Detail index
        uint32_t instanceOffset;    // Start offset in the global instance buffer
        uint32_t maxInstances;      // Window capacity (prevents overflow)
        uint32_t indirectIndex;     // Index in the global indirect draw command buffer
        uint32_t pipelineRenderType; // 0 = Traditional, 1 = Mesh Shader
        uint32_t padding;           // 16-byte std430 alignment
    };
}