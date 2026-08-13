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

#include "SsaoDrawGroup.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Mesh/MeshSourceNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/Source/Procedural/SsaoNoiseImageSource.h"

namespace Syn
{
    SsaoDrawGroup::SsaoDrawGroup(uint32_t frameCount)
    {
        VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        kernelBuffer.Initialize({ "SsaoDrawGroup_KernelBuffer", BufferStrategy::Hybrid, frameCount, sizeof(SsaoKernel), bufferUsage });
        kernelBuffer.UpdateCapacityAll(1);

        std::mt19937 generator;
        std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);

        std::vector<glm::vec4> kernel;
        for (unsigned int i = 0; i < 64; ++i) {
            glm::vec3 sample(
                randomFloats(generator) * 2.0f - 1.0f,
                randomFloats(generator) * 2.0f - 1.0f,
                randomFloats(generator)
            );
            sample = glm::normalize(sample);
            sample *= randomFloats(generator);

            float scale = (float)i / 64.0f;
            scale = 0.1f + (scale * scale) * (1.0f - 0.1f);
            sample *= scale;

            kernel.push_back(glm::vec4(sample, 0.0f));
        }

        for (uint32_t i = 0; i < frameCount; ++i) {
            kernelBuffer.Write(i, kernel.data(), sizeof(SsaoKernel), 0);
        }

		ServiceLocator::Get<ImageManager>()->LoadImageFromSourceSync("SsaoNoiseTexture", []() {
			return std::make_unique<SsaoNoiseImageSource>();
			});
    }

    void SsaoDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) {
        kernelBuffer.RecordSync(cmd, frameIndex);
    }
}