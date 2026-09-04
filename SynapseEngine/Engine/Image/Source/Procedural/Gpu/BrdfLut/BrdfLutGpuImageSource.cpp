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

#include "BrdfLutGpuImageSource.h"
#include "BrdfLutComputePass.h"

namespace Syn
{
    BrdfLutGpuImageSource::BrdfLutGpuImageSource(uint32_t resolution, uint32_t sampleCount)
        : ProceduralImageSource("BrdfLutGpu"), _resolution(resolution), _sampleCount(sampleCount)
    {
        _pipeline = std::make_shared<GpuProceduralPipeline>();
        _pipeline->AddPass(std::make_unique<BrdfLutComputePass>(_sampleCount));
        _pipeline->InitializeAll();
    }

    std::optional<RawImage> BrdfLutGpuImageSource::Produce()
    {
        RawImage image{};
        image.width = _resolution;
        image.height = _resolution;
        image.depth = 1;
        image.mipLevels = 1;
        image.format = VK_FORMAT_R32G32_SFLOAT;
        image.isCompressed = false;
        image.isGpuGenerated = true;
        image.autoCache = true;

        image.gpuGeneratorCallback = [pipeline = _pipeline](VkCommandBuffer cmd, Vk::Image& targetImage) {
            GpuProceduralContext ctx{ cmd, &targetImage };
            pipeline->Execute(ctx);
            };

        return image;
    }
}