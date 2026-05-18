#include "DpHvoPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Image/ImageUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include <glm/glm.hpp>

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/DpHvoPC.glsl"

    bool DpHvoPass::ShouldExecute(const RenderContext& context) const
    {
        auto settings = context.scene->GetSettings();
        return !settings->useDebugCamera;
    }

    void DpHvoPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("DpHvoProgram", {
            ShaderNames::DpHvoComp
            });
    }

    void DpHvoPass::PrepareFrame(const RenderContext& context) {
        auto currGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        auto depthPyramid = currGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto volumetricAo = currGroup->GetImage(RenderTargetNames::VolumetricAo);

        _imageTransitions.push_back({
            depthPyramid,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            false
            });

        _imageTransitions.push_back({
            volumetricAo,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT,
            true
            });
    }

    void DpHvoPass::BindDescriptors(const RenderContext& context) {
        auto currGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto imageManager = ServiceLocator::GetImageManager();

        auto depthPyramid = currGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto volumetricAo = currGroup->GetImage(RenderTargetNames::VolumetricAo);
        auto sampler = imageManager->GetSampler(SamplerNames::LinearClampEdge);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            sampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.AddStorageImage(
            1,
            volumetricAo->GetView(Vk::ImageViewNames::Default),
            VK_IMAGE_LAYOUT_GENERAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void DpHvoPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        uint32_t fIdx = context.frameIndex;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, fIdx);

        DpHvoPC pc{};
        pc.frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx, true);
        pc.aoRadius = scene->GetSettings()->aoRadius;
        pc.aoIntensity = scene->GetSettings()->aoIntensity;
        pc.maxOcclusionDistance = scene->GetSettings()->maxOcclusionDistance;
        pc.bias = scene->GetSettings()->bias;
        pc.sampleCount = scene->GetSettings()->sampleCount;
        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(DpHvoPC), &pc);
    }

    void DpHvoPass::Dispatch(const RenderContext& context) {
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        uint32_t width = rtGroup->GetWidth();
        uint32_t height = rtGroup->GetHeight();

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(width, ComputeGroupSize::Image8D);
        uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(height, ComputeGroupSize::Image8D);

        vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

        auto volumetricAo = rtGroup->GetImage(RenderTargetNames::VolumetricAo);

        volumetricAo->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            VK_ACCESS_2_SHADER_READ_BIT
        );
    }
}