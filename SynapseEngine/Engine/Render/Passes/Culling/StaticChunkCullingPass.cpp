#include "StaticChunkCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/ModelMeshCullingPC.glsl"

    bool StaticChunkCullingPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->enableGeometryGpuCulling
            && context.scene->GetSettings()->enableStaticBvhCulling;
    }

    void StaticChunkCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("StaticChunkCullingProgram", {
            ShaderNames::StaticChunkCulling
            }, config);
    }

    void StaticChunkCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();

        _activeChunkCount = drawData->Chunks.chunkCounter.load(std::memory_order_relaxed);
        if (_activeChunkCount == 0) return;

        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGeometryGpuCulling;

        ModelMeshCullingPC pc{};
        pc.frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx, isGpu);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(ModelMeshCullingPC), &pc);
    }

    void StaticChunkCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();

        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, prevFrameIndex);
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

    void StaticChunkCullingPass::Dispatch(const RenderContext& context) {
        if (_activeChunkCount == 0) return;

        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGeometryGpuCulling;

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_activeChunkCount, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        VkBuffer dispatchBuf = drawData->Chunks.chunkIndirectDispatchBuffer.GetHandle(fIdx, isGpu);
        Vk::BufferBarrierInfo cullBarrier{};
        cullBarrier.buffer = dispatchBuf;
        cullBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        cullBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        cullBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        cullBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, cullBarrier);

        Vk::BufferBarrierInfo chunkBarrier{};
        chunkBarrier.buffer = drawData->Chunks.chunkVisibilityBuffer.GetHandle(fIdx, isGpu);
        chunkBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        chunkBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        chunkBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        chunkBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, chunkBarrier);
    }
}