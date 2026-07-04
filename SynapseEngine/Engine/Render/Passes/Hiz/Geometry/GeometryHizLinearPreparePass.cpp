#include "GeometryHizLinearPreparePass.h"
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
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {
    
    #include "Engine/Shaders/Includes/PushConstants/HizLinearizeDepthPC.glsl"

    bool GeometryHizLinearPreparePass::ShouldExecute(const RenderContext& context) const
    {
        auto settings = context.scene->GetSettings();
        return !settings->debug.useDebugCamera;
    }

    void GeometryHizLinearPreparePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("HizLinearizeDepthProgram", {
            ShaderNames::HizLinearizeDepth
            });
    }

    void GeometryHizLinearPreparePass::PrepareFrame(const RenderContext& context) {
        auto scene = context.scene;

        auto currGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);

        auto depthOpaque = currGroup->GetImage(RenderTargetNames::OpaqueDepth);
        auto depthOpaqueTransparent = currGroup->GetImage(RenderTargetNames::TransparentDepth);
        auto depthPyramid = currGroup->GetImage(RenderTargetNames::DepthPyramid);

        _imageTransitions.push_back({
            depthOpaque,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            false
            });

        _imageTransitions.push_back({
            depthOpaqueTransparent,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            false
            });

        _imageTransitions.push_back({
            depthPyramid,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT,
            true
            });
    }

    void GeometryHizLinearPreparePass::BindDescriptors(const RenderContext& context) {
        auto scene = context.scene;

        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;

        auto imageManager = ServiceLocator::GetImageManager();

        auto currGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);

        auto depthOpaque = currGroup->GetImage(RenderTargetNames::OpaqueDepth);
		auto depthOpaqueTransparent = currGroup->GetImage(RenderTargetNames::TransparentDepth);
        auto depthPyramid = currGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto sampler = imageManager->GetSampler(SamplerNames::NearestClampEdge);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            depthOpaque->GetView(Vk::ImageViewNames::Default),
            sampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.AddCombinedImageSampler(
            1,
            depthOpaqueTransparent->GetView(Vk::ImageViewNames::Default),
            sampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        std::string mip0ViewName = std::string(Vk::ImageViewNames::Default) +
                                   std::string(Vk::ImageViewNames::Mip) +
                                   "0";

        pushWriter.AddStorageImage(
            2,
            depthPyramid->GetView(mip0ViewName),
            VK_IMAGE_LAYOUT_GENERAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void GeometryHizLinearPreparePass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;

        uint32_t fIdx = context.frameIndex;
        auto compManager = scene->GetComponentBufferManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, fIdx);

        Vk::PushConstant<HizLinearizeDepthPC> pc;
		pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx);
        pc->outImageSize = glm::vec2(rtGroup->GetWidth(), rtGroup->GetHeight());    
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void GeometryHizLinearPreparePass::Dispatch(const RenderContext& context) {
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);

        uint32_t width = rtGroup->GetWidth();
        uint32_t height = rtGroup->GetHeight();

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(width, ComputeGroupSize::Image16D);
        uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(height, ComputeGroupSize::Image16D);

        vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);

        depthPyramid->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            VK_ACCESS_2_SHADER_READ_BIT
        );
    }
}