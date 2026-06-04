#include "DirectionLightShadowDrawGroup.h"


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

        instanceBuffer.Initialize({ BufferStrategy::Hybrid_Dynamic, frameCount, sizeof(uint32_t), storageUsage, 16384 * SHADOW_MULTIPLIER, 32768 * SHADOW_MULTIPLIER });
        instanceBuffer.UpdateCapacityAll(1);

        indirectBuffer.Initialize({ BufferStrategy::Hybrid_Dynamic, frameCount, sizeof(VkDrawIndirectCommand) * 8, indirectStorageUsage, 1024, 2048 });
        indirectBuffer.UpdateCapacityAll(1);

        modelDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        modelDispatchBuffer.UpdateCapacityAll(1);

        staticChunkDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        staticChunkDispatchBuffer.UpdateCapacityAll(1);

        mortonChunkDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        mortonChunkDispatchBuffer.UpdateCapacityAll(1);

        Vk::ImageConfig atlasSpec{};
        atlasSpec.width = SHADOW_ATLAS_SIZE;
        atlasSpec.height = SHADOW_ATLAS_SIZE;
        atlasSpec.type = VK_IMAGE_TYPE_2D;
        atlasSpec.format = VK_FORMAT_D32_SFLOAT;
        atlasSpec.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        atlasSpec.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        Vk::ImageConfig hizSpec{};
        hizSpec.width = SHADOW_ATLAS_SIZE;
        hizSpec.height = SHADOW_ATLAS_SIZE;
        hizSpec.type = VK_IMAGE_TYPE_2D;
        hizSpec.format = VK_FORMAT_R32G32_SFLOAT;
        hizSpec.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        hizSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        hizSpec.mipLevels = SHADOW_HIZ_MIP_LEVELS;

        for(int i = 0; i < frameCount; ++i)
            shadowAtlas.push_back(std::make_unique<Vk::Image>(atlasSpec));
            shadowDepthPyramid.push_back(std::make_unique<Vk::Image>(hizSpec));
    }

    void DirectionLightShadowDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) {

    }
}