#include "SsaoBlurPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/SsaoBlurPC.glsl"

    bool SsaoBlurPass::ShouldExecute(const RenderContext& context) const {
        auto settings = context.scene->GetSettings();
        return settings->postProcess.enableSsao && !settings->debug.useDebugCamera;
    }

    void SsaoBlurPass::Initialize() {
        _shaderProgram = ServiceLocator::GetShaderManager()->CreateProgram("SsaoBlurProgram", {
            ShaderNames::SsaoBlurComp
            });
    }

    void SsaoBlurPass::PrepareFrame(const RenderContext& context) {
        auto currGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);

        auto ssaoAo = currGroup->GetImage(RenderTargetNames::SsaoAo);
        auto ssaoAoInt = currGroup->GetImage(RenderTargetNames::SsaoAoIntermediate);

        _imageTransitions.push_back({
            ssaoAoInt,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            false
        });

        _imageTransitions.push_back({
            ssaoAo,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT,
            false
        });
    }

    void SsaoBlurPass::PushConstants(const RenderContext& context)
    {
        Vk::PushConstant<SsaoBlurPC> pc{};
        pc->frameGlobalContextBufferAddr = context.scene->GetSceneDrawData()->frameContextBuffer.GetAddress(context.frameIndex);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SsaoBlurPass::BindDescriptors(const RenderContext& context) {
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        auto imageManager = ServiceLocator::GetImageManager();

        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto ssaoAo = rtGroup->GetImage(RenderTargetNames::SsaoAo);
        auto ssaoAoIntermediate = rtGroup->GetImage(RenderTargetNames::SsaoAoIntermediate);
        auto sampler = imageManager->GetSampler(SamplerNames::LinearClampEdge);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            ssaoAoIntermediate->GetView(Vk::ImageViewNames::Default),
            sampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.AddStorageImage(
            1,
            ssaoAo->GetView(Vk::ImageViewNames::Default),
            VK_IMAGE_LAYOUT_GENERAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void SsaoBlurPass::Dispatch(const RenderContext& context) {
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        auto imageManager = ServiceLocator::GetImageManager();
        auto ssaoAo = rtGroup->GetImage(RenderTargetNames::SsaoAo);

        uint32_t width = rtGroup->GetWidth();
        uint32_t height = rtGroup->GetHeight();
        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(width, ComputeGroupSize::Image8D);
        uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(height, ComputeGroupSize::Image8D);

        vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

        ssaoAo->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            VK_ACCESS_2_SHADER_READ_BIT
        );
    }
}