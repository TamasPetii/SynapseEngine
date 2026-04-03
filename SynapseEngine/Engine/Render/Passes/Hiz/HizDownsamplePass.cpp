#include "HizDownsamplePass.h"
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

    #include "Engine/Shaders/Includes/PushConstants/HizDownSamplePC.glsl"

    bool HizDownsamplePass::ShouldExecute(const RenderContext& context) const
    {
		auto settings = context.scene->GetSettings();
        return settings->enableHiz && settings->enableOcclusionCulling && settings->enableGpuCulling && !settings->useDebugCamera;
    }

    void HizDownsamplePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("HizDownsampleProgram", {
            ShaderNames::HizDownsample
            });
    }

    void HizDownsamplePass::PrepareFrame(const RenderContext& context) {
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);

        _imageTransitions.push_back({
            depthPyramid,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT,
            false
            });
    }

    void HizDownsamplePass::Dispatch(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();
        auto sampler = imageManager->GetSampler(SamplerNames::NearestClampEdge);

        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);

        uint32_t mipLevels = depthPyramid->GetConfig().mipLevels;
        glm::vec2 currentInSize = glm::vec2(rtGroup->GetWidth(), rtGroup->GetHeight());

        Vk::PushDescriptorWriter pushWriter;

        //Skip 0 -> HizLinearPrepass already done it!
        for (uint32_t i = 1; i < mipLevels; ++i) {

            glm::vec2 currentOutSize = glm::vec2(
                std::max(1.0f, std::floor(currentInSize.x / 2.0f)),
                std::max(1.0f, std::floor(currentInSize.y / 2.0f))
            );

            std::string parentMipName = std::string(Vk::ImageViewNames::Default) +
                                        std::string(Vk::ImageViewNames::Mip) +
                                        std::to_string(i - 1);

            std::string currentMipName = std::string(Vk::ImageViewNames::Default) +
                                         std::string(Vk::ImageViewNames::Mip) +
                                         std::to_string(i);

            pushWriter.AddCombinedImageSampler(
                0,
                depthPyramid->GetView(parentMipName),
                sampler->Handle(),
                VK_IMAGE_LAYOUT_GENERAL
            );

            pushWriter.AddStorageImage(
                1,
                depthPyramid->GetView(currentMipName),
                VK_IMAGE_LAYOUT_GENERAL
            );

            pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);

            HizDownSamplePC pc{};
            pc.inImageSize = currentInSize;
            pc.outImageSize = currentOutSize;
            vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(HizDownSamplePC), &pc);

            uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount((uint32_t)currentOutSize.x, ComputeGroupSize::Image16D);
            uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount((uint32_t)currentOutSize.y, ComputeGroupSize::Image16D);

            vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

            Vk::ImageBarrierInfo barrier{};
            barrier.image = depthPyramid->Handle();
            barrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            barrier.srcAccess = VK_ACCESS_2_SHADER_WRITE_BIT;
            barrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            barrier.dstAccess = VK_ACCESS_2_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barrier.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.baseMipLevel = i;
            barrier.levelCount = 1;
            barrier.baseArrayLayer = 0;
            barrier.layerCount = 1;

            Vk::ImageUtils::InsertBarrier(context.cmd, barrier);

            currentInSize = currentOutSize;
        }

        depthPyramid->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT,
            VK_ACCESS_2_SHADER_READ_BIT
        );
    }
}