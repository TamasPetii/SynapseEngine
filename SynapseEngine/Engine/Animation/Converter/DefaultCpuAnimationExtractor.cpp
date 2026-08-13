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

#include "DefaultCpuAnimationExtractor.h"
#include <utility>

namespace Syn
{
    void DefaultCpuAnimationExtractor::Extract(GpuBatchedAnimation& gpuData, CpuAnimationData& outCpuData) const
    {
		outCpuData.descriptor = gpuData.descriptor;
		outCpuData.globalCollider = gpuData.globalCollider;

        outCpuData.frameGlobalColliders = std::move(gpuData.frameGlobalColliders);
        outCpuData.frameMeshColliders = std::move(gpuData.frameMeshColliders);
        outCpuData.tracks = std::move(gpuData.tracks);
        outCpuData.nodes = std::move(gpuData.nodes);
    }
}