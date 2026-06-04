#include "GeometryMortonChunkCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/ModelMeshCullingPC.glsl"

    void GeometryMortonChunkCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("GeometryMortonChunkCullingProgram", {
            ShaderNames::GeometryMortonChunkCullingComp
            }, config);
    }

    bool GeometryMortonChunkCullingPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();
        return context.scene->GetSettings()->enableMortonBvhCulling && pool && !pool->GetStorage().GetStaticEntities().empty();
    }

    void GeometryMortonChunkCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        _staticCount = static_cast<uint32_t>(scene->GetRegistry()->GetPool<TransformComponent>()->GetStorage().GetStaticEntities().size());

        Vk::PushConstant<ModelMeshCullingPC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(context.frameIndex, scene->GetSettings()->enableGeometryGpuCulling);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void GeometryMortonChunkCullingPass::BindDescriptors(const RenderContext& context) {
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

    void GeometryMortonChunkCullingPass::Dispatch(const RenderContext& context) {
        if (_staticCount == 0) return;

        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGeometryGpuCulling;

        VkDispatchIndirectCommand dispatchTemplate = drawData->Chunks.dispatchCmdTemplate;
        VkBuffer modelDispatchBufferHandle = drawData->Chunks.mortonChunkVisibleIndirectDispatchBuffer.GetHandle(fIdx, isGpu);

        Vk::BufferUpdateInfo resetInfo{};
        resetInfo.buffer = modelDispatchBufferHandle;
        resetInfo.offset = 0;
        resetInfo.size = sizeof(VkDispatchIndirectCommand);
        resetInfo.pData = &dispatchTemplate;
        Vk::BufferUtils::UpdateBuffer(context.cmd, resetInfo);

        Vk::BufferBarrierInfo resetBarrier{};
        resetBarrier.buffer = modelDispatchBufferHandle;
        resetBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        resetBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        resetBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        resetBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, resetBarrier);

        VkBuffer chunkIndirectBuffer = drawData->Chunks.mortonIndirectDispatchBuffer.GetHandle(fIdx, isGpu);
        vkCmdDispatchIndirect(context.cmd, chunkIndirectBuffer, 0);

        Vk::BufferBarrierInfo visibleIndexBarrier{};
        visibleIndexBarrier.buffer = scene->GetComponentBufferManager()->GetComponentBuffer(BufferNames::MortonChunkVisibileIndex, fIdx).buffer->Handle();
        visibleIndexBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        visibleIndexBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        visibleIndexBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        visibleIndexBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, visibleIndexBarrier);

        Vk::BufferBarrierInfo modelDispatchBarrier{};
        modelDispatchBarrier.buffer = modelDispatchBufferHandle;
        modelDispatchBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        modelDispatchBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        modelDispatchBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        modelDispatchBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, modelDispatchBarrier);
    }
}