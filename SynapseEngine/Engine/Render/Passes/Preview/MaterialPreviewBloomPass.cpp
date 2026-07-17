#include "MaterialPreviewBloomPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/PreviewManager.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"

#include "Engine/Shaders/Includes/PushConstants/BloomPrefilterPC.glsl"
#include "Engine/Shaders/Includes/PushConstants/BloomDownSamplePC.glsl"
#include "Engine/Shaders/Includes/PushConstants/BloomUpSamplePC.glsl"
#include "Engine/Shaders/Includes/PushConstants/BloomCompositePC.glsl"

namespace Syn {

    bool MaterialPreviewBloomPass::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->postProcess.enableBloom
            && !context.scene->GetSettings()->debug.enableDebugVisibility;
    }

    void MaterialPreviewBloomPass::Initialize() {
        auto sm = ServiceLocator::Get<ShaderManager>();
        _prefilterProgram = sm->CreateProgram("PrevBloomPrefilter", { ShaderNames::BloomPrefilter });
        _downsampleProgram = sm->CreateProgram("PrevBloomDown", { ShaderNames::BloomDownsample });
        _upsampleProgram = sm->CreateProgram("PrevBloomUp", { ShaderNames::BloomUpsample });
        _compositeProgram = sm->CreateProgram("PrevBloomComp", { ShaderNames::BloomComposite });
    }

    void MaterialPreviewBloomPass::Execute(const RenderContext& context) {
        auto pm = ServiceLocator::Get<PreviewManager>();

        std::vector<uint32_t> dirtyMaterials = pm->GetDirtyResources(PreviewResourceType::Material);
        if (dirtyMaterials.empty()) return;

        auto atlas = pm->GetAtlasImage();
        auto scratchColor = pm->GetScratchColorImage();
        auto scratchBloom = pm->GetScratchBloomImage();

        for (uint32_t matId : dirtyMaterials) {

            VkViewport vp; VkRect2D scissor;
            pm->GetViewportAndScissor(PreviewResourceType::Material, matId, vp, scissor);

            // Copy Atlas Region to Scratch Color Texture
            Vk::ImageBarrierInfo atlasToSrc{};
            atlasToSrc.image = atlas->Handle();
            atlasToSrc.srcStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            atlasToSrc.srcAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            atlasToSrc.dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            atlasToSrc.dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT;
            atlasToSrc.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            atlasToSrc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            atlasToSrc.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            atlasToSrc.baseMipLevel = 0;
            atlasToSrc.levelCount = 1;
            atlasToSrc.baseArrayLayer = 0;
            atlasToSrc.layerCount = 1;
            Vk::ImageUtils::InsertBarrier(context.cmd, atlasToSrc);

            Vk::ImageBarrierInfo scratchToDst{};
            scratchToDst.image = scratchColor->Handle();
            scratchToDst.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            scratchToDst.srcAccess = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
            scratchToDst.dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            scratchToDst.dstAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            scratchToDst.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            scratchToDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            scratchToDst.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            scratchToDst.baseMipLevel = 0;
            scratchToDst.levelCount = 1;
            scratchToDst.baseArrayLayer = 0;
            scratchToDst.layerCount = 1;
            Vk::ImageUtils::InsertBarrier(context.cmd, scratchToDst);

            VkImageCopy copyRegion{};
            copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
            copyRegion.srcOffset = { scissor.offset.x, scissor.offset.y, 0 };
            copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
            copyRegion.dstOffset = { 0, 0, 0 };
            copyRegion.extent = { (uint32_t)scissor.extent.width, (uint32_t)scissor.extent.height, 1 };
            vkCmdCopyImage(context.cmd, atlas->Handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, scratchColor->Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

            // Bloom Prefilter
            Vk::ImageBarrierInfo scratchToRead{};
            scratchToRead.image = scratchColor->Handle();
            scratchToRead.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            scratchToRead.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            scratchToRead.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            scratchToRead.dstAccess = VK_ACCESS_2_SHADER_READ_BIT;
            scratchToRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            scratchToRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            scratchToRead.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            scratchToRead.baseMipLevel = 0;
            scratchToRead.levelCount = 1;
            scratchToRead.baseArrayLayer = 0;
            scratchToRead.layerCount = 1;
            Vk::ImageUtils::InsertBarrier(context.cmd, scratchToRead);

            Vk::ImageBarrierInfo bloomSync{};
            bloomSync.image = scratchBloom->Handle();
            bloomSync.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            bloomSync.srcAccess = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
            bloomSync.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            bloomSync.dstAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
            bloomSync.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            bloomSync.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            bloomSync.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            bloomSync.baseMipLevel = 0;
            bloomSync.levelCount = 1;
            bloomSync.baseArrayLayer = 0;
            bloomSync.layerCount = 1;
            Vk::ImageUtils::InsertBarrier(context.cmd, bloomSync);

            DispatchPrefilter(context, scratchColor, scratchBloom);

            // Bloom down and up sample simulation
            DispatchDownsample(context, scratchBloom);
            DispatchUpsample(context, scratchBloom);

            // Bloom scratch composite pass
            Vk::ImageBarrierInfo scratchToGen{};
            scratchToGen.image = scratchColor->Handle();
            scratchToGen.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            scratchToGen.srcAccess = VK_ACCESS_2_SHADER_READ_BIT;
            scratchToGen.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            scratchToGen.dstAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
            scratchToGen.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            scratchToGen.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            scratchToGen.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            scratchToGen.baseMipLevel = 0;
            scratchToGen.levelCount = 1;
            scratchToGen.baseArrayLayer = 0;
            scratchToGen.layerCount = 1;
            Vk::ImageUtils::InsertBarrier(context.cmd, scratchToGen);

            DispatchComposite(context, scratchColor, scratchBloom);

            // Copy Scratch to Atlas Region
            Vk::ImageBarrierInfo scratchToSrc{};
            scratchToSrc.image = scratchColor->Handle();
            scratchToSrc.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            scratchToSrc.srcAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
            scratchToSrc.dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            scratchToSrc.dstAccess = VK_ACCESS_2_TRANSFER_READ_BIT;
            scratchToSrc.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            scratchToSrc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            scratchToSrc.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            scratchToSrc.baseMipLevel = 0;
            scratchToSrc.levelCount = 1;
            scratchToSrc.baseArrayLayer = 0;
            scratchToSrc.layerCount = 1;
            Vk::ImageUtils::InsertBarrier(context.cmd, scratchToSrc);

            Vk::ImageBarrierInfo atlasToDst{};
            atlasToDst.image = atlas->Handle();
            atlasToDst.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            atlasToDst.srcAccess = VK_ACCESS_2_TRANSFER_READ_BIT;
            atlasToDst.dstStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            atlasToDst.dstAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            atlasToDst.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            atlasToDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            atlasToDst.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            atlasToDst.baseMipLevel = 0;
            atlasToDst.levelCount = 1;
            atlasToDst.baseArrayLayer = 0;
            atlasToDst.layerCount = 1;
            Vk::ImageUtils::InsertBarrier(context.cmd, atlasToDst);

            VkImageCopy copyBackRegion{};
            copyBackRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
            copyBackRegion.srcOffset = { 0, 0, 0 };
            copyBackRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
            copyBackRegion.dstOffset = { scissor.offset.x, scissor.offset.y, 0 };
            copyBackRegion.extent = { (uint32_t)scissor.extent.width, (uint32_t)scissor.extent.height, 1 };
            vkCmdCopyImage(context.cmd, scratchColor->Handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, atlas->Handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyBackRegion);

            // Post transition to prepare for rendering or iteration
            Vk::ImageBarrierInfo atlasToColor{};
            atlasToColor.image = atlas->Handle();
            atlasToColor.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            atlasToColor.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            atlasToColor.dstStage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            atlasToColor.dstAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            atlasToColor.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            atlasToColor.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            atlasToColor.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            atlasToColor.baseMipLevel = 0;
            atlasToColor.levelCount = 1;
            atlasToColor.baseArrayLayer = 0;
            atlasToColor.layerCount = 1;
            Vk::ImageUtils::InsertBarrier(context.cmd, atlasToColor);

            // Scratch Color vissza Generalra a következő iterációnak
            Vk::ImageBarrierInfo scratchBackToGen{};
            scratchBackToGen.image = scratchColor->Handle();
            scratchBackToGen.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            scratchBackToGen.srcAccess = VK_ACCESS_2_TRANSFER_READ_BIT;
            scratchBackToGen.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            scratchBackToGen.dstAccess = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
            scratchBackToGen.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            scratchBackToGen.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            scratchBackToGen.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            scratchBackToGen.baseMipLevel = 0;
            scratchBackToGen.levelCount = 1;
            scratchBackToGen.baseArrayLayer = 0;
            scratchBackToGen.layerCount = 1;
            Vk::ImageUtils::InsertBarrier(context.cmd, scratchBackToGen);
        }

        pm->ClearDirtyResources(PreviewResourceType::Material);
    }

    void MaterialPreviewBloomPass::DispatchPrefilter(const RenderContext& context, Vk::Image* colorImage, Vk::Image* bloomImage) {
        _prefilterProgram->Bind(context.cmd);

        auto sampler = ServiceLocator::Get<ImageManager>()->GetSampler(SamplerNames::LinearClampEdge);

        Vk::PushDescriptorWriter writer;
        writer.AddCombinedImageSampler(0, colorImage->GetView(Vk::ImageViewNames::Default), sampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        writer.AddStorageImage(1, bloomImage->GetView(std::string(Vk::ImageViewNames::Default) + std::string(Vk::ImageViewNames::Mip) + "0"), VK_IMAGE_LAYOUT_GENERAL);
        writer.Push(context.cmd, _prefilterProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);

        Vk::PushConstant<BloomPrefilterPC> pc;
        pc->knee = context.scene->GetSettings()->postProcess.bloomKnee;
        pc->threshold = context.scene->GetSettings()->postProcess.bloomThreshold;
        pc->texelSize = 1.0f / glm::vec2(colorImage->GetExtent().width, colorImage->GetExtent().height);
        pc.Push(context.cmd, _prefilterProgram->GetLayout());

        uint32_t gx = ComputeGroupSize::CalculateDispatchCount(colorImage->GetExtent().width, ComputeGroupSize::Image8D);
        uint32_t gy = ComputeGroupSize::CalculateDispatchCount(colorImage->GetExtent().height, ComputeGroupSize::Image8D);
        vkCmdDispatch(context.cmd, gx, gy, 1);
    }

    void MaterialPreviewBloomPass::DispatchDownsample(const RenderContext& context, Vk::Image* bloomImage) {
        _downsampleProgram->Bind(context.cmd);

        auto sampler = ServiceLocator::Get<ImageManager>()->GetSampler(SamplerNames::LinearClampEdge);
        Vk::PushDescriptorWriter pushWriter;
        glm::vec2 currentInSize = glm::vec2(bloomImage->GetExtent().width, bloomImage->GetExtent().height);

        for (uint32_t i = 1; i < bloomImage->GetConfig().mipLevels; ++i) {
            glm::vec2 currentOutSize = glm::vec2(std::max(1.0f, std::floor(currentInSize.x / 2.0f)), std::max(1.0f, std::floor(currentInSize.y / 2.0f)));
            std::string parentMip = std::string(Vk::ImageViewNames::Default) + std::string(Vk::ImageViewNames::Mip) + std::to_string(i - 1);
            std::string currentMip = std::string(Vk::ImageViewNames::Default) + std::string(Vk::ImageViewNames::Mip) + std::to_string(i);

            Vk::ImageBarrierInfo barrier{};
            barrier.image = bloomImage->Handle();
            barrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            barrier.srcAccess = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
            barrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            barrier.dstAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.baseMipLevel = i;
            barrier.levelCount = 1;
            barrier.baseArrayLayer = 0;
            barrier.layerCount = 1;
            Vk::ImageUtils::InsertBarrier(context.cmd, barrier);

            pushWriter.AddCombinedImageSampler(0, bloomImage->GetView(parentMip), sampler->Handle(), VK_IMAGE_LAYOUT_GENERAL);
            pushWriter.AddStorageImage(1, bloomImage->GetView(currentMip), VK_IMAGE_LAYOUT_GENERAL);
            pushWriter.Push(context.cmd, _downsampleProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);

            Vk::PushConstant<BloomDownSamplePC> pc;
            pc->texelSize = 1.0f / currentInSize;
            pc.Push(context.cmd, _downsampleProgram->GetLayout());

            uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount((uint32_t)currentOutSize.x, ComputeGroupSize::Image8D);
            uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount((uint32_t)currentOutSize.y, ComputeGroupSize::Image8D);
            vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

            Vk::ImageBarrierInfo readBarrier{};
            readBarrier.image = bloomImage->Handle();
            readBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            readBarrier.srcAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
            readBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            readBarrier.dstAccess = VK_ACCESS_2_SHADER_READ_BIT;
            readBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            readBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            readBarrier.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            readBarrier.baseMipLevel = i;
            readBarrier.levelCount = 1;
            readBarrier.baseArrayLayer = 0;
            readBarrier.layerCount = 1;
            Vk::ImageUtils::InsertBarrier(context.cmd, readBarrier);

            currentInSize = currentOutSize;
        }
    }

    void MaterialPreviewBloomPass::DispatchUpsample(const RenderContext& context, Vk::Image* bloomImage) {
        _upsampleProgram->Bind(context.cmd);

        auto sampler = ServiceLocator::Get<ImageManager>()->GetSampler(SamplerNames::LinearClampEdge);
        Vk::PushDescriptorWriter pushWriter;
        glm::vec2 baseSize = glm::vec2(bloomImage->GetExtent().width, bloomImage->GetExtent().height);

        for (int32_t i = bloomImage->GetConfig().mipLevels - 1; i > 0; --i) {
            glm::vec2 sourceSize = glm::vec2(std::max(1.0f, (float)std::floor(baseSize.x / std::pow(2.0f, i))), std::max(1.0f, (float)std::floor(baseSize.y / std::pow(2.0f, i))));
            glm::vec2 targetSize = glm::vec2(std::max(1.0f, (float)std::floor(baseSize.x / std::pow(2.0f, i - 1))), std::max(1.0f, (float)std::floor(baseSize.y / std::pow(2.0f, i - 1))));

            std::string sourceMip = std::string(Vk::ImageViewNames::Default) + std::string(Vk::ImageViewNames::Mip) + std::to_string(i);
            std::string targetMip = std::string(Vk::ImageViewNames::Default) + std::string(Vk::ImageViewNames::Mip) + std::to_string(i - 1);

            pushWriter.AddCombinedImageSampler(0, bloomImage->GetView(sourceMip), sampler->Handle(), VK_IMAGE_LAYOUT_GENERAL);
            pushWriter.AddStorageImage(1, bloomImage->GetView(targetMip), VK_IMAGE_LAYOUT_GENERAL);
            pushWriter.Push(context.cmd, _upsampleProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);

            Vk::PushConstant<BloomUpSamplePC> pc;
            pc->texelSize = 1.0f / sourceSize;
            pc->filterRadius = context.scene->GetSettings()->postProcess.bloomFilterRadius;
            pc.Push(context.cmd, _upsampleProgram->GetLayout());

            uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount((uint32_t)targetSize.x, ComputeGroupSize::Image8D);
            uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount((uint32_t)targetSize.y, ComputeGroupSize::Image8D);
            vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

            Vk::ImageBarrierInfo barrier{};
            barrier.image = bloomImage->Handle();
            barrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            barrier.srcAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
            barrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            barrier.dstAccess = VK_ACCESS_2_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.baseMipLevel = i - 1;
            barrier.levelCount = 1;
            barrier.baseArrayLayer = 0;
            barrier.layerCount = 1;
            Vk::ImageUtils::InsertBarrier(context.cmd, barrier);
        }
    }

    void MaterialPreviewBloomPass::DispatchComposite(const RenderContext& context, Vk::Image* colorImage, Vk::Image* bloomImage) {
        _compositeProgram->Bind(context.cmd);
        
        auto sampler = ServiceLocator::Get<ImageManager>()->GetSampler(SamplerNames::LinearClampEdge);
        Vk::PushDescriptorWriter writer;
        writer.AddCombinedImageSampler(0, bloomImage->GetView(std::string(Vk::ImageViewNames::Default) + std::string(Vk::ImageViewNames::Mip) + "0"), sampler->Handle(), VK_IMAGE_LAYOUT_GENERAL);
        writer.AddStorageImage(1, colorImage->GetView(Vk::ImageViewNames::Default), VK_IMAGE_LAYOUT_GENERAL);
        writer.Push(context.cmd, _compositeProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);

        Vk::PushConstant<BloomCompositePC> pc;
        pc->exposure = context.scene->GetSettings()->postProcess.bloomExposure;
        pc->bloomStrength = context.scene->GetSettings()->postProcess.bloomStrength;
        pc.Push(context.cmd, _compositeProgram->GetLayout());

        uint32_t gx = ComputeGroupSize::CalculateDispatchCount(colorImage->GetExtent().width, ComputeGroupSize::Image8D);
        uint32_t gy = ComputeGroupSize::CalculateDispatchCount(colorImage->GetExtent().height, ComputeGroupSize::Image8D);
        vkCmdDispatch(context.cmd, gx, gy, 1);
    }
}