#include "DpHvoBlurPass.h"
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

#include "Engine/Shaders/Includes/PushConstants/DpHvoBlurPC.glsl"

    bool DpHvoBlurPass::ShouldExecute(const RenderContext& context) const {
        return !context.scene->GetSettings()->useDebugCamera;
    }

    void DpHvoBlurPass::Initialize() {
        _shaderProgram = ServiceLocator::GetShaderManager()->CreateProgram("DpHvoBlurProgram", {
            ShaderNames::DpHvoBlurComp
            });
    }

    void DpHvoBlurPass::PrepareFrame(const RenderContext& context) {
        auto currGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        auto ssaoAo = currGroup->GetImage(RenderTargetNames::SsaoAo);
        auto ssaocAoInt = currGroup->GetImage(RenderTargetNames::SsaoAoIntermediate);

        _imageTransitions.push_back({
            ssaoAo,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            false
            });

        _imageTransitions.push_back({
            ssaocAoInt,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT,
            false
            });
    }

    void DpHvoBlurPass::Dispatch(const RenderContext& context) {
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto imageManager = ServiceLocator::GetImageManager();

        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto ssaoAo = rtGroup->GetImage(RenderTargetNames::SsaoAo);
        auto ssaoAoInt = rtGroup->GetImage(RenderTargetNames::SsaoAoIntermediate);

        auto sampler = imageManager->GetSampler(SamplerNames::LinearClampEdge);

        uint32_t width = rtGroup->GetWidth();
        uint32_t height = rtGroup->GetHeight();
        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(width, ComputeGroupSize::Image8D);
        uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(height, ComputeGroupSize::Image8D);

        Vk::PushConstant<DpHvoBlurPC> pc{};
        pc->frameGlobalContextBufferAddr = context.scene->GetSceneDrawData()->frameContextBuffer.GetAddress(context.frameIndex, true);
        pc->depthSharpness = context.scene->GetSettings()->depthSharpness;

        Vk::PushDescriptorWriter pushWriterH;
        pushWriterH.AddCombinedImageSampler(0, ssaoAo->GetView(Vk::ImageViewNames::Default), sampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        pushWriterH.AddCombinedImageSampler(1, depthPyramid->GetView(Vk::ImageViewNames::Default), sampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        pushWriterH.AddStorageImage(2, ssaoAoInt->GetView(Vk::ImageViewNames::Default), VK_IMAGE_LAYOUT_GENERAL);
        pushWriterH.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);

        pc->blurDirection = glm::vec2(1.0f, 0.0f);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
        vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

        ssaoAoInt->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);
        ssaoAo->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_WRITE_BIT);

        Vk::PushDescriptorWriter pushWriterV;
        pushWriterV.AddCombinedImageSampler(0, ssaoAoInt->GetView(Vk::ImageViewNames::Default), sampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        pushWriterV.AddCombinedImageSampler(1, depthPyramid->GetView(Vk::ImageViewNames::Default), sampler->Handle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        pushWriterV.AddStorageImage(2, ssaoAo->GetView(Vk::ImageViewNames::Default), VK_IMAGE_LAYOUT_GENERAL);
        pushWriterV.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);

        pc->blurDirection = glm::vec2(0.0f, 1.0f);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
        vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

        ssaoAo->TransitionLayout(context.cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_READ_BIT);
    }
}