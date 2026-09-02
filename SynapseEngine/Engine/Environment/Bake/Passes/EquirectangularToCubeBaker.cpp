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

#include "EquirectangularToCubeBaker.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Render/ShaderNames.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn {
    void EquirectangularToCubeBaker::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();

        _shaderProgramId = shaderManager->LoadProgramSync("EquirectangularToCube", { 
            ShaderNames::EquirectangularToCube
        });
    }

    void EquirectangularToCubeBaker::BindDescriptors(const EnvironmentBakeContext& context) 
    {
        auto sampler = ServiceLocator::Get<ImageManager>()->GetSampler(SamplerNames::LinearClampEdge);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            context.sourceEquirectangular->image->GetView(Vk::ImageViewNames::Default),
            sampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.AddStorageImage(
            1,
            context.baseCubemap->image->GetView(Vk::ImageViewNames::Default),
            VK_IMAGE_LAYOUT_GENERAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void EquirectangularToCubeBaker::Dispatch(const EnvironmentBakeContext& context) {
        uint32_t width = context.baseCubemap->image->GetConfig().width;
        uint32_t groupCount = ComputeGroupSize::CalculateDispatchCount(width, ComputeGroupSize::Image16D);
        vkCmdDispatch(context.cmd, groupCount, groupCount, 6);
    }
}