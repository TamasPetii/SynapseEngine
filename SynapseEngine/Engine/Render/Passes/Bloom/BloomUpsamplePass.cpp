#include "BloomUpsamplePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Image/ImageManager.h"
#include <glm/glm.hpp>
#include <algorithm>

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/BloomUpSamplePC.glsl"

    void BloomUpsamplePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("BloomUpsampleProgram", {
            ShaderNames::BloomUpsample
            });
    }

    void BloomUpsamplePass::PrepareFrame(const RenderContext& context) {
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto bloom = rtGroup->GetImage(RenderTargetNames::Bloom);

        _imageTransitions.push_back({
            bloom,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
            false
            });
    }

    void BloomUpsamplePass::Dispatch(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();
        auto sampler = imageManager->GetSampler(SamplerNames::LinearClampEdge);

        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto bloom = rtGroup->GetImage(RenderTargetNames::Bloom);

        uint32_t mipLevels = bloom->GetConfig().mipLevels;
        glm::vec2 baseSize = glm::vec2(rtGroup->GetWidth(), rtGroup->GetHeight());

        Vk::PushDescriptorWriter pushWriter;

        for (int32_t i = mipLevels - 1; i > 0; --i) {
            glm::vec2 sourceSize = glm::vec2(
                std::max(1.0f, (float)std::floor(baseSize.x / std::pow(2.0f, i))),
                std::max(1.0f, (float)std::floor(baseSize.y / std::pow(2.0f, i)))
            );

            glm::vec2 targetSize = glm::vec2(
                std::max(1.0f, (float)std::floor(baseSize.x / std::pow(2.0f, i - 1))),
                std::max(1.0f, (float)std::floor(baseSize.y / std::pow(2.0f, i - 1)))
            );

            std::string sourceMipName = 
                std::string(Vk::ImageViewNames::Default) +
                std::string(Vk::ImageViewNames::Mip) +
                std::to_string(i);

            std::string targetMipName = 
                std::string(Vk::ImageViewNames::Default) +
                std::string(Vk::ImageViewNames::Mip) +
                std::to_string(i - 1);

            pushWriter.AddCombinedImageSampler(
                0,
                bloom->GetView(sourceMipName),
                sampler->Handle(),
                VK_IMAGE_LAYOUT_GENERAL
            );

            pushWriter.AddStorageImage(
                1,
                bloom->GetView(targetMipName),
                VK_IMAGE_LAYOUT_GENERAL
            );

            pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);

            BloomUpSamplePC pc{};
            pc.texelSize = 1.0f / sourceSize;
            pc.filterRadius = _config.filterRadius;
            vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(BloomUpSamplePC), &pc);

            uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount((uint32_t)targetSize.x, ComputeGroupSize::Image8D);
            uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount((uint32_t)targetSize.y, ComputeGroupSize::Image8D);

            vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

            Vk::ImageBarrierInfo barrier{};
            barrier.image = bloom->Handle();
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
}