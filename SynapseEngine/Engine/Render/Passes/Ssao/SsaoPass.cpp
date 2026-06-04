#include "SsaoPass.h"
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
#include "Engine/Image/ImageNames.h"
#include "Engine/Render/ComputeGroupSize.h"
#include <glm/glm.hpp>
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/SsaoPC.glsl"

    bool SsaoPass::ShouldExecute(const RenderContext& context) const
    {
        auto settings = context.scene->GetSettings();
        return settings->enableSsao && !settings->useDebugCamera;
    }

    void SsaoPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("SsaoProgram", {
            ShaderNames::SsaoComp
            });
    }

    void SsaoPass::PrepareFrame(const RenderContext& context) {

    }

    void SsaoPass::BindDescriptors(const RenderContext& context) {
        auto currGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto imageManager = ServiceLocator::GetImageManager();

        auto noiseTexture = imageManager->GetResource(ImageNames::SsaoNoiseTexture);
        auto depthPyramid = currGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto ssaoAoIntermediate = currGroup->GetImage(RenderTargetNames::SsaoAoIntermediate);
        auto sampler = imageManager->GetSampler(SamplerNames::LinearClampEdge);
		auto samplerRepeat = imageManager->GetSampler(SamplerNames::LinearRepeat);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            sampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.AddCombinedImageSampler(
            1,
            noiseTexture->image->GetView(Vk::ImageViewNames::Default),
            samplerRepeat->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.AddStorageImage(
            2,
            ssaoAoIntermediate->GetView(Vk::ImageViewNames::Default),
            VK_IMAGE_LAYOUT_GENERAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void SsaoPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        uint32_t fIdx = context.frameIndex;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, fIdx);

        auto imageManager = ServiceLocator::GetImageManager();
        auto noiseTexture = imageManager->GetResource(ImageNames::SsaoNoiseTexture);

        Vk::PushConstant<SsaoPC> pc{};
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx, true);
        pc->aoRadius = scene->GetSettings()->aoRadius;
        pc->aoIntensity = scene->GetSettings()->aoIntensity;
        pc->maxOcclusionDistance = scene->GetSettings()->maxOcclusionDistance;
        pc->bias = scene->GetSettings()->bias;
        pc->sampleCount = scene->GetSettings()->sampleCount;
		pc->noiseTextureWidth = static_cast<float>(noiseTexture->image->GetExtent().width);
		pc->noiseTextureHeight = static_cast<float>(noiseTexture->image->GetExtent().height);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SsaoPass::Dispatch(const RenderContext& context) {
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        uint32_t width = rtGroup->GetWidth();
        uint32_t height = rtGroup->GetHeight();

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(width, ComputeGroupSize::Image8D);
        uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(height, ComputeGroupSize::Image8D);

        vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);
    }
}