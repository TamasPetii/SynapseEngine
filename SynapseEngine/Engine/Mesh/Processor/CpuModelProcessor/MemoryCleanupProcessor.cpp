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

#include "MemoryCleanupProcessor.h"

namespace Syn
{
    void MemoryCleanupProcessor::Process(CpuModelData& cpuData)
    {
        cpuData.worldPositions.clear();
        cpuData.worldPositions.shrink_to_fit();

        //This is needed for animation!

        /*
        cpuData.vertices.clear();
        cpuData.vertices.shrink_to_fit();
       
        cpuData.indices.clear();
        cpuData.indices.shrink_to_fit();

        cpuData.batchedIndicesPerLod.clear();
        cpuData.batchedIndicesPerLod.shrink_to_fit();

        cpuData.meshletVertexIndices->clear();
        cpuData.meshletVertexIndices->shrink_to_fit();

        cpuData.meshletTriangleIndices->clear();
        cpuData.meshletTriangleIndices->shrink_to_fit();

        cpuData.meshletDescriptors->clear();
        cpuData.meshletDescriptors->shrink_to_fit();
        */
    }
}