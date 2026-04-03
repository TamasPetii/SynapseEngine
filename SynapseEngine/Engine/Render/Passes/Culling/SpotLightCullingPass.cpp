#include "SpotLightCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/SpotLightComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/SpotLightCullingPC.glsl"

    bool SpotLightCullingPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->enableGpuCulling;
    }

    void SpotLightCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("SpotLightCullingProgram", {
            ShaderNames::SpotLightCulling
            }, { .useDescriptorBuffers = false });
    }

    void SpotLightCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto registry = scene->GetRegistry();
        auto spotLightPool = registry->GetPool<SpotLightComponent>();
        _totalLightsToTest = spotLightPool ? static_cast<uint32_t>(spotLightPool->Size()) : 0;

        if (_totalLightsToTest == 0) return;

        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        SpotLightCullingPC pc{};
        pc.cameraBufferAddr = compManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapBufferAddr = compManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.spotLightColliderDataAddr = compManager->GetBufferAddr(BufferNames::SpotLightColliderData, fIdx);
        pc.visibleLightAddr = compManager->GetBufferAddr(BufferNames::SpotLightVisibleData, fIdx);
        pc.indirectCommandAddr = drawData->spotLightIndirectCommandBuffers[fIdx]->GetDeviceAddress();
        pc.totalLightsToTest = _totalLightsToTest;
        pc.activeCameraEntity = scene->GetSceneCameraEntity();
        pc.enableOcclusionCulling = scene->GetSettings()->enableOcclusionCulling ? 1 : 0;

        pc.screenWidth = static_cast<float>(rtGroup->GetWidth());
        pc.screenHeight = static_cast<float>(rtGroup->GetHeight());

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(SpotLightCullingPC), &pc);
    }

    void SpotLightCullingPass::BindDescriptors(const RenderContext& context) {
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

    void SpotLightCullingPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        if (_totalLightsToTest == 0) return;

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_totalLightsToTest, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        Vk::BufferBarrierInfo cmdBarrier{};
        cmdBarrier.buffer = drawData->spotLightIndirectCommandBuffers[fIdx]->Handle();
        cmdBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        cmdBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        cmdBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        cmdBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_TRANSFER_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, cmdBarrier);

        Vk::BufferBarrierInfo visibleBarrier{};
        visibleBarrier.buffer = compManager->GetComponentBuffer(BufferNames::SpotLightVisibleData, fIdx).buffer->Handle();
        visibleBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        visibleBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        visibleBarrier.dstStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        visibleBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, visibleBarrier);
    }
}