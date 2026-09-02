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

#include "DirectionLightShadowDrawGroup.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Render/RenderNames.h"

namespace Syn
{
    DirectionLightShadowDrawGroup::DirectionLightShadowDrawGroup(uint32_t frameCount)
    {
        dispatchCmdTemplate.x = 0;
        dispatchCmdTemplate.y = 1;
        dispatchCmdTemplate.z = 1;

        paddedTraditionalCounts.AssignZero(16);
        paddedMeshletCounts.AssignZero(16);

        instances.AssignZero(1);

        VkBufferUsageFlags storageUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VkBufferUsageFlags indirectStorageUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        instanceBuffer.Initialize({ "DirectionLightShadowDrawGroup_InstanceBuffer", BufferStrategy::Hybrid, frameCount, sizeof(uint32_t), storageUsage, 16384 * SHADOW_MULTIPLIER, 32768 * SHADOW_MULTIPLIER});
        instanceBuffer.UpdateCapacityAll(1);

        indirectBuffer.Initialize({ "DirectionLightShadowDrawGroup_IndirectBuffer", BufferStrategy::Hybrid, frameCount, sizeof(VkDrawIndirectCommand) * MaterialRenderType::MaterialRenderTypeCount * 2, indirectStorageUsage, 1024, 2048 });
        indirectBuffer.UpdateCapacityAll(1);

        modelDispatchBuffer.Initialize({ "DirectionLightShadowDrawGroup_ModelDispatchBuffer", BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        modelDispatchBuffer.UpdateCapacityAll(1);

        staticChunkDispatchBuffer.Initialize({ "DirectionLightShadowDrawGroup_StaticChunkDispatchBuffer", BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        staticChunkDispatchBuffer.UpdateCapacityAll(1);

        mortonChunkDispatchBuffer.Initialize({ "DirectionLightShadowDrawGroup_MortonChunkDispatchBuffer", BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        mortonChunkDispatchBuffer.UpdateCapacityAll(1);

        Vk::ImageConfig atlasSpec{};
        atlasSpec.width = SHADOW_ATLAS_SIZE;
        atlasSpec.height = SHADOW_ATLAS_SIZE;
        atlasSpec.type = VK_IMAGE_TYPE_2D;
        atlasSpec.format = VK_FORMAT_D32_SFLOAT;
        atlasSpec.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        atlasSpec.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        Vk::ImageConfig hizSpec{};
        hizSpec.width = SHADOW_ATLAS_SIZE;
        hizSpec.height = SHADOW_ATLAS_SIZE;
        hizSpec.type = VK_IMAGE_TYPE_2D;
        hizSpec.format = VK_FORMAT_R32G32_SFLOAT;
        hizSpec.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        hizSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        hizSpec.mipLevels = SHADOW_HIZ_MIP_LEVELS;

        hizSpec.AddView(Vk::ImageViewNames::Default, Vk::ImageViewConfig{
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .perMipViews = true
            });

        hizSpec.AddView(RenderTargetViewNames::DirectionLightShadowDepthPyramidMax, Vk::ImageViewConfig{
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .swizzle = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ONE },
                    .perMipViews = true
            });

        hizSpec.AddView(RenderTargetViewNames::DirectionLightShadowDepthPyramidMin, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .swizzle = { VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ONE },
            .perMipViews = true
            });

        Vk::ImageConfig colorAtlasSpec{};
        colorAtlasSpec.width = SHADOW_ATLAS_SIZE;
        colorAtlasSpec.height = SHADOW_ATLAS_SIZE;
        colorAtlasSpec.type = VK_IMAGE_TYPE_2D;
        colorAtlasSpec.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        colorAtlasSpec.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        colorAtlasSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        colorAtlasSpec.AddView(Vk::ImageViewNames::Default, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D
            });

        colorAtlasSpec.AddView(RenderTargetViewNames::DirectionLightShadowTransparentColor, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .swizzle = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_ONE }
            });

        colorAtlasSpec.AddView(RenderTargetViewNames::DirectionLightShadowTransparentDepth, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .swizzle = { VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_ONE }
            });

        for (int i = 0; i < frameCount; ++i) {
            shadowAtlas.push_back(std::make_unique<Vk::Image>(atlasSpec));
            shadowDepthPyramid.push_back(std::make_unique<Vk::Image>(hizSpec));
            shadowColorAtlas.push_back(std::make_unique<Vk::Image>(colorAtlasSpec));
            staticShadowAtlas.push_back(std::make_unique<Vk::Image>(atlasSpec));
            staticShadowColorAtlas.push_back(std::make_unique<Vk::Image>(colorAtlasSpec));
        }
    }

    void DirectionLightShadowDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) {
        indirectBuffer.RecordSync(cmd, frameIndex);
        instanceBuffer.RecordSync(cmd, frameIndex);
    }
}