#include "ObjectCuller.h"
#include "Engine/Render/GpuStructs.h"
#include "Engine/Components/DefaultColliderComponent.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/SpotLightComponent.h"

void ObjectCuller::Initialize(std::shared_ptr<ResourceManager> resourceManager)
{
}

void ObjectCuller::Render(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, std::function<void()> renderFunction)
{
    CullObjectInCameraFrustum(commandBuffer, registry, resourceManager, frameIndex);
    CullSpotLightInCameraFrustum(commandBuffer, registry, resourceManager, frameIndex);
    CullPointLightInCameraFrustum(commandBuffer, registry, resourceManager, frameIndex);

    CullPointLightShadowAabb(commandBuffer, registry, resourceManager, frameIndex);
    CullPointLightShadowObjects(commandBuffer, registry, resourceManager, frameIndex);

    CullSpotLightShadowAabb(commandBuffer, registry, resourceManager, frameIndex);
    CullSpotLightShadowObjects(commandBuffer, registry, resourceManager, frameIndex);

    {
        VkMemoryBarrier2 drawBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
        drawBarrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        drawBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        drawBarrier.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        drawBarrier.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT;

        VkDependencyInfo drawDepInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        drawDepInfo.memoryBarrierCount = 1;
        drawDepInfo.pMemoryBarriers = &drawBarrier;

        vkCmdPipelineBarrier2(commandBuffer, &drawDepInfo);
    }
}

void ObjectCuller::CullObjectInCameraFrustum(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
    auto vulkanContext = Vk::VulkanContext::GetContext();
    auto device = vulkanContext->GetDevice();
    auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingCameraFrustum");
    auto pushDescriptor = resourceManager->GetVulkanManager()->GetPushDescriptorSet("DepthPyramid");

    auto previousFrameIndex = (frameIndex + GlobalConfig::FrameConfig::framesInFlight - 1) % GlobalConfig::FrameConfig::framesInFlight;
    auto previousFrameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", previousFrameIndex);

    auto modelIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(resourceManager->GetModelManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());
    auto shapeIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(resourceManager->GetGeometryManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());

    for (uint32_t i = 0; i < resourceManager->GetModelManager()->GetCurrentCount(); ++i)
        modelIndirectDrawBufferHandler[i].instanceCount = 0;

    for (uint32_t i = 0; i < resourceManager->GetGeometryManager()->GetCurrentCount(); ++i)
        shapeIndirectDrawBufferHandler[i].instanceCount = 0;

    if (!registry->GetPool<DefaultColliderComponent>())
        return;

    uint32_t defaultColliderCount = registry->GetPool<DefaultColliderComponent>()->GetDenseSize();

    uint32_t workgroupSize = 64;
    uint32_t workgroupCount = static_cast<uint32_t>(std::ceil(defaultColliderCount / (float)workgroupSize));

    if (workgroupCount == 0)
        return;

    CullingCameraFrustumPushConstants pushConstants;
    pushConstants.cameraIndex = 0;
    pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
    pushConstants.cameraFrustumBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraFrustumData", frameIndex)->buffer->GetAddress();

    pushConstants.defaultColliderCount = defaultColliderCount;
    pushConstants.defaultColliderBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderData", frameIndex)->buffer->GetAddress();

    pushConstants.modelRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ModelRenderIndicesData", frameIndex)->buffer->GetAddress();
    pushConstants.modelBufferAddresses = resourceManager->GetModelManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.modelDrawIndirectCommandBuffer = resourceManager->GetModelManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.modelInstanceIndexAddressBuffer = resourceManager->GetModelManager()->GetInstanceIndexAddressBuffer(frameIndex)->buffer->GetAddress();

    pushConstants.shapeRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ShapeRenderIndicesData", frameIndex)->buffer->GetAddress();
    pushConstants.shapeBufferAddresses = resourceManager->GetGeometryManager()->GetDeviceAddressesBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.shapeDrawIndirectCommandBuffer = resourceManager->GetGeometryManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.shapeInstanceIndexAddressBuffer = resourceManager->GetGeometryManager()->GetInstanceIndexAddressBuffer(frameIndex)->buffer->GetAddress();

    pushConstants.depthPyramidSize = glm::vec2(previousFrameBuffer->GetSize().width, previousFrameBuffer->GetSize().height);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
    vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingCameraFrustumPushConstants), &pushConstants);

    VkDescriptorImageInfo srcTarget;
    srcTarget.sampler = resourceManager->GetVulkanManager()->GetSampler("MaxReduction")->Value();
    srcTarget.imageView = previousFrameBuffer->GetImage("DepthPyramid")->GetImageView("Default");
    srcTarget.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    std::vector<VkWriteDescriptorSet> descriptorWrites =
    {
        Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("SrcImage"), srcTarget)
    };

    device->vkCmdPushDescriptorSetKHR(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetLayout(), 0, descriptorWrites.size(), descriptorWrites.data());

    vkCmdDispatch(commandBuffer, workgroupCount, 1, 1);
}

void ObjectCuller::CullPointLightInCameraFrustum(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
    auto vulkanContext = Vk::VulkanContext::GetContext();
    auto device = vulkanContext->GetDevice();
    auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingPointLight");
    auto pushDescriptor = resourceManager->GetVulkanManager()->GetPushDescriptorSet("DepthPyramid");
    auto previousFrameIndex = (frameIndex + GlobalConfig::FrameConfig::framesInFlight - 1) % GlobalConfig::FrameConfig::framesInFlight;
    auto previousFrameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", previousFrameIndex);

    auto pointLightShadowCountBufferHandler = static_cast<LightBufferCommonData*>(resourceManager->GetPointLightBufferManager()->GetCommonDataBuffer(frameIndex)->buffer->GetHandler());
    pointLightShadowCountBufferHandler[0] = LightBufferCommonData{};

    auto pointLightShadowDispatchIndirectBuffers = static_cast<VkDispatchIndirectCommand*>(resourceManager->GetPointLightBufferManager()->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetHandler());
    pointLightShadowDispatchIndirectBuffers[0] = VkDispatchIndirectCommand{
        .x = 1,
        .y = 1,
        .z = 1
    };

    auto pointLightIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(resourceManager->GetPointLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());
    pointLightIndirectDrawBufferHandler[0] = VkDrawIndirectCommand{
        .vertexCount = resourceManager->GetGeometryManager()->GetShape("Cube")->GetIndexCount(),
        .instanceCount = 0,
        .firstVertex = 0,
        .firstInstance = 0
    };

    if (!registry->GetPool<PointLightComponent>())
        return;

    uint32_t pointLightCount = registry->GetPool<PointLightComponent>()->GetDenseSize();

    uint32_t workgroupSize = 16;
    uint32_t workgroupCount = static_cast<uint32_t>(std::ceil(pointLightCount / (float)workgroupSize));

    if (workgroupCount == 0)
        return;

    CullingPointLightPushConstants pushConstants;
    pushConstants.cameraIndex = 0;
    pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
    pushConstants.cameraFrustumBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraFrustumData", frameIndex)->buffer->GetAddress();
    pushConstants.pointLightCount = pointLightCount;
    pushConstants.pointLightBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightData", frameIndex)->buffer->GetAddress();
    pushConstants.pointLightInstanceIndexBuffer = resourceManager->GetPointLightBufferManager()->GetInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.pointLightDrawIndirectCommandBuffer = resourceManager->GetPointLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.pointLightShadowInstanceIndexBuffer = resourceManager->GetPointLightBufferManager()->GetShadowInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.pointLightCommonDataBuffer = resourceManager->GetPointLightBufferManager()->GetCommonDataBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.pointLightShadowDispatchIndirectBuffer = resourceManager->GetPointLightBufferManager()->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetAddress();
    pushConstants.colliderDebugBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightColliderDebug", frameIndex)->buffer->GetAddress();
    pushConstants.depthPyramidSize = glm::vec2(previousFrameBuffer->GetSize().width, previousFrameBuffer->GetSize().height);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
    vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingPointLightPushConstants), &pushConstants);

    VkDescriptorImageInfo srcTarget;
    srcTarget.sampler = resourceManager->GetVulkanManager()->GetSampler("MaxReduction")->Value();
    srcTarget.imageView = previousFrameBuffer->GetImage("DepthPyramid")->GetImageView("Default");
    srcTarget.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    std::vector<VkWriteDescriptorSet> descriptorWrites =
    {
        Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("SrcImage"), srcTarget)
    };

    device->vkCmdPushDescriptorSetKHR(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetLayout(), 0, descriptorWrites.size(), descriptorWrites.data());

    vkCmdDispatch(commandBuffer, workgroupCount, 1, 1);
}

void ObjectCuller::CullSpotLightInCameraFrustum(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
    auto vulkanContext = Vk::VulkanContext::GetContext();
    auto device = vulkanContext->GetDevice();
    auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingSpotLight");
    auto pushDescriptor = resourceManager->GetVulkanManager()->GetPushDescriptorSet("DepthPyramid");
    auto previousFrameIndex = (frameIndex + GlobalConfig::FrameConfig::framesInFlight - 1) % GlobalConfig::FrameConfig::framesInFlight;
    auto previousFrameBuffer = resourceManager->GetVulkanManager()->GetFrameDependentFrameBuffer("Main", previousFrameIndex);

    auto spotLightShadowCountBufferHandler = static_cast<LightBufferCommonData*>(resourceManager->GetSpotLightBufferManager()->GetCommonDataBuffer(frameIndex)->buffer->GetHandler());
    spotLightShadowCountBufferHandler[0] = LightBufferCommonData{};

    auto spotLightShadowDispatchIndirectBuffers = static_cast<VkDispatchIndirectCommand*>(resourceManager->GetSpotLightBufferManager()->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetHandler());
    spotLightShadowDispatchIndirectBuffers[0] = VkDispatchIndirectCommand{
        .x = 1,
        .y = 1,
        .z = 1
    };

    auto spotLightIndirectDrawBufferHandler = static_cast<VkDrawIndirectCommand*>(resourceManager->GetSpotLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetHandler());
    spotLightIndirectDrawBufferHandler[0] = VkDrawIndirectCommand{
        .vertexCount = resourceManager->GetGeometryManager()->GetShape("Cone")->GetIndexCount(),
        .instanceCount = 0,
        .firstVertex = 0,
        .firstInstance = 0
    };

    if (!registry->GetPool<SpotLightComponent>())
        return;

    uint32_t spotLightCount = registry->GetPool<SpotLightComponent>()->GetDenseSize();

    uint32_t workgroupSize = 16;
    uint32_t workgroupCount = static_cast<uint32_t>(std::ceil(spotLightCount / (float)workgroupSize));

    if (workgroupCount == 0)
        return;

    CullingSpotLightPushConstants pushConstants;
    pushConstants.cameraIndex = 0;
    pushConstants.cameraBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraData", frameIndex)->buffer->GetAddress();
    pushConstants.cameraFrustumBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("CameraFrustumData", frameIndex)->buffer->GetAddress();
    pushConstants.spotLightCount = spotLightCount;
    pushConstants.spotLightBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightData", frameIndex)->buffer->GetAddress();
    pushConstants.spotLightInstanceIndexBuffer = resourceManager->GetSpotLightBufferManager()->GetInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.spotLightDrawIndirectCommandBuffer = resourceManager->GetSpotLightBufferManager()->GetIndirectDrawBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.spotLightShadowInstanceIndexBuffer = resourceManager->GetSpotLightBufferManager()->GetShadowInstanceIndexBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.spotLightCommonDataBuffer = resourceManager->GetSpotLightBufferManager()->GetCommonDataBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.spotLightShadowDispatchIndirectBuffer = resourceManager->GetSpotLightBufferManager()->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetAddress();
    pushConstants.colliderDebugBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightColliderDebug", frameIndex)->buffer->GetAddress();
    pushConstants.depthPyramidSize = glm::vec2(previousFrameBuffer->GetSize().width, previousFrameBuffer->GetSize().height);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
    vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingSpotLightPushConstants), &pushConstants);

    VkDescriptorImageInfo srcTarget;
    srcTarget.sampler = resourceManager->GetVulkanManager()->GetSampler("MaxReduction")->Value();
    srcTarget.imageView = previousFrameBuffer->GetImage("DepthPyramid")->GetImageView("Default");
    srcTarget.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    std::vector<VkWriteDescriptorSet> descriptorWrites =
    {
        Vk::DesciptorSetUtils::BuildWritePushDescriptorSetInfo(pushDescriptor->GetImageLayout("SrcImage"), srcTarget)
    };

    device->vkCmdPushDescriptorSetKHR(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetLayout(), 0, descriptorWrites.size(), descriptorWrites.data());

    vkCmdDispatch(commandBuffer, workgroupCount, 1, 1);
}

void ObjectCuller::CullPointLightShadowAabb(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
    {
        VkMemoryBarrier2 syncShadowData = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
        syncShadowData.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        syncShadowData.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        syncShadowData.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        syncShadowData.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

        VkDependencyInfo depInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        depInfo.memoryBarrierCount = 1;
        depInfo.pMemoryBarriers = &syncShadowData;

        vkCmdPipelineBarrier2(commandBuffer, &depInfo);
    }

    if (!registry->GetPool<DefaultColliderComponent>())
        return;

    uint32_t defaultColliderCount = registry->GetPool<DefaultColliderComponent>()->GetDenseSize();
    uint32_t workgroupSize = 64;
    uint32_t workgroupCount = static_cast<uint32_t>(std::ceil(defaultColliderCount / (float)workgroupSize));

    if (workgroupCount == 0)
        return;

    auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingLightShadowAabb");
    auto pointLightBufferManager = resourceManager->GetPointLightBufferManager();

    CullingLightShadowAabbPushConstants pushConstants;
    pushConstants.defaultColliderCount = defaultColliderCount;
    pushConstants.defaultColliderBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderData", frameIndex)->buffer->GetAddress();
    pushConstants.defaultColliderInstanceIndexBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderIndexPointLight", frameIndex)->buffer->GetAddress();
    pushConstants.lightCommonDataBuffer = pointLightBufferManager->GetCommonDataBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.lightShadowDispatchIndirectBuffer = pointLightBufferManager->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetAddress();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
    vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingLightShadowAabbPushConstants), &pushConstants);
    vkCmdDispatch(commandBuffer, workgroupCount, 1, 1);
}

void ObjectCuller::CullPointLightShadowObjects(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
    {
        VkMemoryBarrier2 syncDispatch = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
        syncDispatch.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        syncDispatch.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        syncDispatch.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        syncDispatch.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT;

        VkDependencyInfo depInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        depInfo.memoryBarrierCount = 1;
        depInfo.pMemoryBarriers = &syncDispatch;

        vkCmdPipelineBarrier2(commandBuffer, &depInfo);
    }

    if (!registry->GetPool<DefaultColliderComponent>())
        return;

    //if (!registry->GetPool<PointLightComponent>() || registry->GetPool<PointLightComponent>()->GetDenseSize() == 0)
        //return;

    auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingPointLightShadowObjects");
    auto pointLightBufferManager = resourceManager->GetPointLightBufferManager();
    auto pointLightShadowBufferManager = resourceManager->GetPointLightShadowBufferManager();

    CullingPointLightsShadowObjectsPushConstants pushConstants;
    pushConstants.defaultColliderBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderData", frameIndex)->buffer->GetAddress();
    pushConstants.defaultColliderCulledIndexBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderIndexPointLight", frameIndex)->buffer->GetAddress();

    pushConstants.pointLightBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("PointLightData", frameIndex)->buffer->GetAddress();
    pushConstants.pointLightCommonDataBuffer = pointLightBufferManager->GetCommonDataBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.pointLightShadowInstanceIndexBuffer = pointLightBufferManager->GetShadowInstanceIndexBuffer(frameIndex)->buffer->GetAddress();

    pushConstants.modelRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ModelRenderIndicesData", frameIndex)->buffer->GetAddress();
    pushConstants.pointLightShadowModelInstanceBufferAddresses = pointLightShadowBufferManager->GetModelInstanceAddressBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.pointLightShadowModelIndirectDrawBufferAddresses = pointLightShadowBufferManager->GetModelIndirectAddressBuffer(frameIndex)->buffer->GetAddress();

    pushConstants.shapeRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ShapeRenderIndicesData", frameIndex)->buffer->GetAddress();
    pushConstants.pointLightShadowShapeInstanceBufferAddresses = pointLightShadowBufferManager->GetShapeInstanceAddressBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.pointLightShadowShapeIndirectDrawBufferAddresses = pointLightShadowBufferManager->GetShapeIndirectAddressBuffer(frameIndex)->buffer->GetAddress();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
    vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingPointLightsShadowObjectsPushConstants), &pushConstants);

    vkCmdDispatchIndirect(commandBuffer, pointLightBufferManager->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->Value(), 0);
}

void ObjectCuller::CullSpotLightShadowAabb(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
    {
        VkMemoryBarrier2 syncShadowData = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
        syncShadowData.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        syncShadowData.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        syncShadowData.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        syncShadowData.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;

        VkDependencyInfo depInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        depInfo.memoryBarrierCount = 1;
        depInfo.pMemoryBarriers = &syncShadowData;

        vkCmdPipelineBarrier2(commandBuffer, &depInfo);
    }

    if (!registry->GetPool<DefaultColliderComponent>())
        return;

    uint32_t defaultColliderCount = registry->GetPool<DefaultColliderComponent>()->GetDenseSize();
    uint32_t workgroupSize = 64;
    uint32_t workgroupCount = static_cast<uint32_t>(std::ceil(defaultColliderCount / (float)workgroupSize));

    if (workgroupCount == 0)
        return;

    auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingLightShadowAabb");
    auto spotLightBufferManager = resourceManager->GetSpotLightBufferManager();

    CullingLightShadowAabbPushConstants pushConstants;
    pushConstants.defaultColliderCount = defaultColliderCount;
    pushConstants.defaultColliderBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderData", frameIndex)->buffer->GetAddress();
    pushConstants.defaultColliderInstanceIndexBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderIndexSpotLight", frameIndex)->buffer->GetAddress();
    pushConstants.lightCommonDataBuffer = spotLightBufferManager->GetCommonDataBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.lightShadowDispatchIndirectBuffer = spotLightBufferManager->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->GetAddress();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
    vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingLightShadowAabbPushConstants), &pushConstants);
    vkCmdDispatch(commandBuffer, workgroupCount, 1, 1);
}

void ObjectCuller::CullSpotLightShadowObjects(VkCommandBuffer commandBuffer, std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex)
{
    {
        VkMemoryBarrier2 syncDispatch = { VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
        syncDispatch.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        syncDispatch.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT;
        syncDispatch.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        syncDispatch.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_READ_BIT;

        VkDependencyInfo depInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
        depInfo.memoryBarrierCount = 1;
        depInfo.pMemoryBarriers = &syncDispatch;

        vkCmdPipelineBarrier2(commandBuffer, &depInfo);
    }

    if (!registry->GetPool<DefaultColliderComponent>())
        return;

    auto pipeline = resourceManager->GetVulkanManager()->GetComputePipeline("CullingSpotLightShadowObjects");
    auto spotLightBufferManager = resourceManager->GetSpotLightBufferManager();
    auto spotLightShadowBufferManager = resourceManager->GetSpotLightShadowBufferManager();

    CullingSpotLightsShadowObjectsPushConstants pushConstants;
    pushConstants.defaultColliderBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderData", frameIndex)->buffer->GetAddress();
    pushConstants.defaultColliderCulledIndexBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("DefaultColliderIndexSpotLight", frameIndex)->buffer->GetAddress();

    pushConstants.spotLightBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("SpotLightData", frameIndex)->buffer->GetAddress();
    pushConstants.spotLightCommonDataBuffer = spotLightBufferManager->GetCommonDataBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.spotLightShadowInstanceIndexBuffer = spotLightBufferManager->GetShadowInstanceIndexBuffer(frameIndex)->buffer->GetAddress();

    pushConstants.modelRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ModelRenderIndicesData", frameIndex)->buffer->GetAddress();
    pushConstants.spotLightShadowModelInstanceBufferAddresses = spotLightShadowBufferManager->GetModelInstanceAddressBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.spotLightShadowModelIndirectDrawBufferAddresses = spotLightShadowBufferManager->GetModelIndirectAddressBuffer(frameIndex)->buffer->GetAddress();

    pushConstants.shapeRenderIndicesBuffer = resourceManager->GetComponentBufferManager()->GetComponentBuffer("ShapeRenderIndicesData", frameIndex)->buffer->GetAddress();
    pushConstants.spotLightShadowShapeInstanceBufferAddresses = spotLightShadowBufferManager->GetShapeInstanceAddressBuffer(frameIndex)->buffer->GetAddress();
    pushConstants.spotLightShadowShapeIndirectDrawBufferAddresses = spotLightShadowBufferManager->GetShapeIndirectAddressBuffer(frameIndex)->buffer->GetAddress();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->GetPipeline());
    vkCmdPushConstants(commandBuffer, pipeline->GetLayout(), VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingSpotLightsShadowObjectsPushConstants), &pushConstants);
    vkCmdDispatchIndirect(commandBuffer, spotLightBufferManager->GetShadowDispatchIndirectBuffers(frameIndex)->buffer->Value(), 0);
}