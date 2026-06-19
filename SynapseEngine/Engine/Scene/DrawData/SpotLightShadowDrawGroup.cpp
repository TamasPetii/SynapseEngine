#include "SpotLightShadowDrawGroup.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Render/RenderNames.h"

namespace Syn
{
    SpotLightShadowDrawGroup::SpotLightShadowDrawGroup(uint32_t frameCount)
    {
        dispatchCmdTemplate.x = 0;
        dispatchCmdTemplate.y = 1;
        dispatchCmdTemplate.z = 1;

        VkBufferUsageFlags storageUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VkBufferUsageFlags indirectStorageUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        instanceBuffer.Initialize({ BufferStrategy::Hybrid_Dynamic, frameCount, sizeof(SpotShadowInstancePayload), storageUsage, 65536, 131072 });
        instanceBuffer.UpdateCapacityAll(1);

        indirectBuffer.Initialize({ BufferStrategy::Hybrid_Dynamic, frameCount, sizeof(VkDrawIndirectCommand) * 8, indirectStorageUsage, 1024, 2048 });
        indirectBuffer.UpdateCapacityAll(1);

        descriptorBuffer.Initialize({ BufferStrategy::Hybrid_Dynamic, frameCount, sizeof(MeshDrawDescriptor) * 8, storageUsage, 1024, 2048 });
        descriptorBuffer.UpdateCapacityAll(1);

        modelDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        modelDispatchBuffer.UpdateCapacityAll(1);

        staticChunkDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        staticChunkDispatchBuffer.UpdateCapacityAll(1);

        mortonChunkDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        mortonChunkDispatchBuffer.UpdateCapacityAll(1);

        Vk::ImageConfig atlasSpec{};
        atlasSpec.width = SPOT_SHADOW_ATLAS_SIZE;
        atlasSpec.height = SPOT_SHADOW_ATLAS_SIZE;
        atlasSpec.type = VK_IMAGE_TYPE_2D;
        atlasSpec.format = VK_FORMAT_D32_SFLOAT;
        atlasSpec.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        atlasSpec.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        Vk::ImageConfig hizSpec{};
        hizSpec.width = SPOT_SHADOW_ATLAS_SIZE;
        hizSpec.height = SPOT_SHADOW_ATLAS_SIZE;
        hizSpec.type = VK_IMAGE_TYPE_2D;
        hizSpec.format = VK_FORMAT_R32G32_SFLOAT;
        hizSpec.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        hizSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        hizSpec.mipLevels = SPOT_SHADOW_HIZ_MIP_LEVELS;

        hizSpec.AddView(Vk::ImageViewNames::Default, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .perMipViews = true
        });

        hizSpec.AddView(RenderTargetViewNames::SpotLightShadowDepthPyramidMax, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .swizzle = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ONE },
            .perMipViews = true
        });

        hizSpec.AddView(RenderTargetViewNames::SpotLightShadowDepthPyramidMin, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .swizzle = { VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ONE },
            .perMipViews = true
        });

        for (int i = 0; i < frameCount; ++i) {
            shadowAtlas.push_back(std::make_unique<Vk::Image>(atlasSpec));
            shadowDepthPyramid.push_back(std::make_unique<Vk::Image>(hizSpec));
        }
    }

    void SpotLightShadowDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) {
        if (totalCommandCount > 0) {
            indirectBuffer.RecordSync(cmd, frameIndex, totalCommandCount);
        }
    }
}