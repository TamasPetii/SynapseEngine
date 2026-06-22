#include "PointLightShadowDrawGroup.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Render/RenderNames.h"

namespace Syn
{
    PointLightShadowDrawGroup::PointLightShadowDrawGroup(uint32_t frameCount)
    {
        dispatchCmdTemplate.x = 0;
        dispatchCmdTemplate.y = 1;
        dispatchCmdTemplate.z = 1;

        VkBufferUsageFlags storageUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VkBufferUsageFlags indirectStorageUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        instanceBuffer.Initialize({ BufferStrategy::Hybrid, frameCount, sizeof(PointShadowInstancePayload), storageUsage, 65536, 131072 });
        instanceBuffer.UpdateCapacityAll(1);

        unsortedInstanceBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(PointShadowInstancePayload), storageUsage, 65536, 131072 });
        unsortedInstanceBuffer.UpdateCapacityAll(1);

        sortValuesBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(uint32_t), storageUsage, 65536, 131072 });
        sortValuesBuffer.UpdateCapacityAll(1);

        drawCallKeyBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(uint32_t), storageUsage, 65536, 131072 });
        drawCallKeyBuffer.UpdateCapacityAll(1);

        indirectBuffer.Initialize({ BufferStrategy::Hybrid, frameCount, sizeof(VkDrawIndirectCommand) * 8, indirectStorageUsage, 1024, 2048 });
        indirectBuffer.UpdateCapacityAll(1);

        visibleCountDispatchBuffer.Initialize({ BufferStrategy::Hybrid, frameCount, sizeof(uint32_t), storageUsage, 1, 1 });
        visibleCountDispatchBuffer.UpdateCapacityAll(1);

        visibleMeshCountDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(uint32_t), storageUsage, 1, 1 });
        visibleMeshCountDispatchBuffer.UpdateCapacityAll(1);

        descriptorBuffer.Initialize({ BufferStrategy::Hybrid, frameCount, sizeof(MeshDrawDescriptor) * 8, storageUsage, 1024, 2048 });
        descriptorBuffer.UpdateCapacityAll(1);

        modelCullingIndirectDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        modelCullingIndirectDispatchBuffer.UpdateCapacityAll(1);

        modelDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        modelDispatchBuffer.UpdateCapacityAll(1);

        finalizeDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        finalizeDispatchBuffer.UpdateCapacityAll(1);

        staticChunkDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        staticChunkDispatchBuffer.UpdateCapacityAll(1);

        mortonChunkDispatchBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, sizeof(VkDispatchIndirectCommand), indirectStorageUsage, 1, 1 });
        mortonChunkDispatchBuffer.UpdateCapacityAll(1);

        radixSortTempBuffer.Initialize({ BufferStrategy::GpuOnly, frameCount, 1, storageUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 10000, 20000 });
        radixSortTempBuffer.UpdateCapacityAll(1);

        gridLookupData.Resize(POINT_SHADOW_GRID_SIZE * POINT_SHADOW_GRID_SIZE);
        std::fill(gridLookupData.Data(), gridLookupData.Data() + (POINT_SHADOW_GRID_SIZE * POINT_SHADOW_GRID_SIZE), 0xFFFFFFFF);

        gridLookupBuffer.Initialize({ BufferStrategy::Hybrid, frameCount, sizeof(uint32_t) * POINT_SHADOW_GRID_SIZE * POINT_SHADOW_GRID_SIZE, storageUsage, 1, 1 });
        gridLookupBuffer.UpdateCapacityAll(1);

        Vk::ImageConfig atlasSpec{};
        atlasSpec.width = POINT_SHADOW_ATLAS_SIZE;
        atlasSpec.height = POINT_SHADOW_ATLAS_SIZE;
        atlasSpec.type = VK_IMAGE_TYPE_2D;
        atlasSpec.format = VK_FORMAT_D32_SFLOAT;
        atlasSpec.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        atlasSpec.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        Vk::ImageConfig hizSpec{};
        hizSpec.width = POINT_SHADOW_ATLAS_SIZE;
        hizSpec.height = POINT_SHADOW_ATLAS_SIZE;
        hizSpec.type = VK_IMAGE_TYPE_2D;
        hizSpec.format = VK_FORMAT_R32G32_SFLOAT;
        hizSpec.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        hizSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        hizSpec.mipLevels = POINT_SHADOW_HIZ_MIP_LEVELS;

        hizSpec.AddView(Vk::ImageViewNames::Default, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .perMipViews = true
            });

        hizSpec.AddView(RenderTargetViewNames::PointLightShadowDepthPyramidMax, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .swizzle = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ONE },
            .perMipViews = true
            });

        hizSpec.AddView(RenderTargetViewNames::PointLightShadowDepthPyramidMin, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .swizzle = { VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ONE },
            .perMipViews = true
            });

        for (int i = 0; i < frameCount; ++i) {
            shadowAtlas.push_back(std::make_unique<Vk::Image>(atlasSpec));
            shadowDepthPyramid.push_back(std::make_unique<Vk::Image>(hizSpec));
        }
    }

    void PointLightShadowDrawGroup::CoherentToGpuBufferSync(VkCommandBuffer cmd, uint32_t frameIndex) {
        indirectBuffer.RecordSync(cmd, frameIndex);
        descriptorBuffer.RecordSync(cmd, frameIndex);
        instanceBuffer.RecordSync(cmd, frameIndex);
        gridLookupBuffer.RecordSync(cmd, frameIndex);
        visibleCountDispatchBuffer.RecordSync(cmd, frameIndex);
    }
}