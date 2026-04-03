#include "PointLightCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/PointLightComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/PointLightCullingPC.glsl"

    bool PointLightCullingPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->enableGpuCulling;
    }

    void PointLightCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("PointLightCullingProgram", {
            ShaderNames::PointLightCulling
            }, config);
    }

    void PointLightCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;

        auto registry = scene->GetRegistry();
        auto pointLightPool = registry->GetPool<PointLightComponent>();
        _totalLightsToTest = pointLightPool ? static_cast<uint32_t>(pointLightPool->Size()) : 0;

        if (_totalLightsToTest == 0) return;

        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        PointLightCullingPC pc{};
        pc.cameraBufferAddr = compManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.pointLightColliderDataAddr = compManager->GetBufferAddr(BufferNames::PointLightColliderData, fIdx);
        pc.visibleLightAddr = compManager->GetBufferAddr(BufferNames::PointLightVisibleData, fIdx);
        pc.indirectCommandAddr = drawData->pointLightIndirectCommandBuffers[fIdx]->GetDeviceAddress();
        pc.totalLightsToTest = _totalLightsToTest;
        pc.activeCameraEntity = scene->GetSceneCameraEntity();
        pc.enableOcclusionCulling = scene->GetSettings()->enableOcclusionCulling ? 1 : 0;

        pc.screenWidth = static_cast<float>(rtGroup->GetWidth());
        pc.screenHeight = static_cast<float>(rtGroup->GetHeight());

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(PointLightCullingPC), &pc);
    }

    void PointLightCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        Vk::PushDescriptorWriter pushWriter;
        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            maxSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void PointLightCullingPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        if (_totalLightsToTest == 0) return;

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_totalLightsToTest, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo cmdBarrier{};
        cmdBarrier.buffer = drawData->pointLightIndirectCommandBuffers[fIdx]->Handle();
        cmdBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        cmdBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        cmdBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        cmdBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_TRANSFER_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, cmdBarrier);

        Vk::BufferBarrierInfo visibleDataBarrier{};
        visibleDataBarrier.buffer = compManager->GetComponentBuffer(BufferNames::PointLightVisibleData, fIdx).buffer->Handle();
        visibleDataBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        visibleDataBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        visibleDataBarrier.dstStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        visibleDataBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, visibleDataBarrier);
    }
}