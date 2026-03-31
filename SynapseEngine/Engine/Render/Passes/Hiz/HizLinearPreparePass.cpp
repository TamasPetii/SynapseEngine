#include "HizLinearPreparePass.h"
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
    
    #include "Engine/Shaders/Includes/PushConstants/HizLinearizeDepthPC.glsl"

    void HizLinearPreparePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("HizLinearizeDepthProgram", {
            ShaderNames::HizLinearizeDepth
            });
    }

    void HizLinearPreparePass::PrepareFrame(const RenderContext& context) {
        auto scene = context.scene;

        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;

        auto prevGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, prevFrameIndex);
        auto currGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        auto depthRaw = prevGroup->GetImage(RenderTargetNames::Depth);
        auto depthPyramid = currGroup->GetImage(RenderTargetNames::DepthPyramid);

        _imageTransitions.push_back({
            depthRaw,
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

    void HizLinearPreparePass::BindDescriptors(const RenderContext& context) {
        auto scene = context.scene;

        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;

        auto imageManager = ServiceLocator::GetImageManager();
        auto prevGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, prevFrameIndex);
        auto currGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        auto depthRaw = prevGroup->GetImage(RenderTargetNames::Depth);
        auto depthPyramid = currGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto sampler = imageManager->GetSampler(SamplerNames::NearestClampEdge);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            depthRaw->GetView(Vk::ImageViewNames::Default),
            sampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        std::string mip0ViewName = std::string(Vk::ImageViewNames::Default) +
                                   std::string(Vk::ImageViewNames::Mip) +
                                   "0";

        pushWriter.AddStorageImage(
            1,
            depthPyramid->GetView(mip0ViewName),
            VK_IMAGE_LAYOUT_GENERAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void HizLinearPreparePass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;

        uint32_t fIdx = context.frameIndex;
        auto compManager = scene->GetComponentBufferManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, fIdx);

        HizLinearizeDepthPC pc{};
        pc.cameraBufferAddr = compManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.activeCameraEntity = scene->GetSceneCameraEntity();
        pc.outImageSize = glm::vec2(rtGroup->GetWidth(), rtGroup->GetHeight());

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(HizLinearizeDepthPC), &pc);
    }

    void HizLinearPreparePass::Dispatch(const RenderContext& context) {
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

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