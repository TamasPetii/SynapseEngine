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

#include "DefaultEnvironmentUploader.h"
#include "Engine/Environment/Bake/Passes/EquirectangularToCubeBaker.h"
#include "Engine/Environment/Bake/Passes/IrradianceBaker.h"
#include "Engine/Environment/Bake/Passes/PrefilterBaker.h"
#include "Engine/Vk/Buffer/BufferFactory.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include <cmath>

namespace Syn 
{
    DefaultEnvironmentUploader::DefaultEnvironmentUploader() {
        _pipeline.AddBaker(std::make_unique<EquirectangularToCubeBaker>());
        _pipeline.AddBaker(std::make_unique<IrradianceBaker>());
        _pipeline.AddBaker(std::make_unique<PrefilterBaker>());
        _pipeline.InitializeAll();
    }

    EnvironmentUploadResult DefaultEnvironmentUploader::Upload(Environment& data, VkCommandBuffer cmd, Vk::GpuUploader* uploader) {
        EnvironmentUploadResult result;

        if (!data.transientSourceImage || !data.transientSourceImage->image) {
            return result;
        }

        uint32_t baseRes = 1024;
        uint32_t baseMipLevels = static_cast<uint32_t>(std::floor(std::log2(baseRes))) + 1;

        Vk::ImageConfig baseConfig{};
        baseConfig.width = baseRes;
        baseConfig.height = baseRes;
        baseConfig.depth = 1;
        baseConfig.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        baseConfig.mipLevels = baseMipLevels;
        baseConfig.arrayLayers = 6;
        baseConfig.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        baseConfig.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

        baseConfig.AddView(Vk::ImageViewNames::Default, {
            .viewType = VK_IMAGE_VIEW_TYPE_CUBE,
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .perMipViews = true
            });

        result.baseCubemap = std::make_shared<Texture>();
        result.baseCubemap->image = std::make_shared<Vk::Image>(baseConfig);

        Vk::ImageConfig irradianceConfig = baseConfig;
        irradianceConfig.width = 32;
        irradianceConfig.height = 32;
        irradianceConfig.mipLevels = 1;

        result.irradianceMap = std::make_shared<Texture>();
        result.irradianceMap->image = std::make_shared<Vk::Image>(irradianceConfig);

        Vk::ImageConfig prefilterConfig = baseConfig;
        prefilterConfig.width = 128;
        prefilterConfig.height = 128;
        uint32_t prefilterMips = static_cast<uint32_t>(std::floor(std::log2(128))) + 1;
        prefilterConfig.mipLevels = prefilterMips;

        result.prefilteredMap = std::make_shared<Texture>();
        result.prefilteredMap->image = std::make_shared<Vk::Image>(prefilterConfig);

        result.baseCubemap->image->TransitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, true);
        result.irradianceMap->image->TransitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, true);
        result.prefilteredMap->image->TransitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT, true);
        //data.transientSourceImage->image->TransitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT, false);

        EnvironmentBakeContext ctx{
            .cmd = cmd,
            .sourceEquirectangular = data.transientSourceImage.get(),
            .baseCubemap = result.baseCubemap.get(),
            .irradianceMap = result.irradianceMap.get(),
            .prefilteredMap = result.prefilteredMap.get()
        };

        _pipeline.Execute(ctx);

        result.baseCubemap->image->TransitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, 0, false);
        result.irradianceMap->image->TransitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, 0, false);
        result.prefilteredMap->image->TransitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, 0, false);

        uploader->RegisterImageTransfer({ result.baseCubemap->image->Handle(), VK_IMAGE_ASPECT_COLOR_BIT, baseMipLevels, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
        uploader->RegisterImageTransfer({ result.irradianceMap->image->Handle(), VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
        uploader->RegisterImageTransfer({ result.prefilteredMap->image->Handle(), VK_IMAGE_ASPECT_COLOR_BIT, prefilterMips, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });

        return result;
    }
}