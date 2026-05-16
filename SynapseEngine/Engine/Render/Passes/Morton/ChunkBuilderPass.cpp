#include "ChunkBuilderPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/ChunkBuilderPC.glsl"

    void ChunkBuilderPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("ChunkBuilderProgram", { 
            ShaderNames::ChunkBuilder 
        }, config);
    }

    bool ChunkBuilderPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();
        return context.scene->GetSettings()->enableMortonBvhCulling && pool && !pool->GetStorage().GetStaticEntities().empty();
    }

    void ChunkBuilderPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        _staticCount = static_cast<uint32_t>(scene->GetRegistry()->GetPool<TransformComponent>()->GetStorage().GetStaticEntities().size());

        ChunkBuilderPC pc{};
        pc.frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(context.frameIndex, scene->GetSettings()->enableGeometryGpuCulling);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(ChunkBuilderPC), &pc);
    }

    void ChunkBuilderPass::Dispatch(const RenderContext& context) {
        if (_staticCount == 0) return;

        auto scene = context.scene;
        auto compManager = scene->GetComponentBufferManager();
        auto drawGroup = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGeometryGpuCulling;

        VkDrawIndirectCommand drawTemplate = drawGroup->Chunks.wireframeCmdTemplate;

        Vk::BufferUpdateInfo drawUpdateInfo{};
        drawUpdateInfo.buffer = drawGroup->Chunks.mortonIndirectDrawBuffer.GetHandle(fIdx, isGpu);
        drawUpdateInfo.offset = 0;
        drawUpdateInfo.size = sizeof(VkDrawIndirectCommand);
        drawUpdateInfo.pData = &drawTemplate;
        Vk::BufferUtils::UpdateBuffer(context.cmd, drawUpdateInfo);
            
        Vk::BufferBarrierInfo drawBarrier{};
        drawBarrier.buffer = drawUpdateInfo.buffer;
        drawBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        drawBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        drawBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        drawBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, drawBarrier);

        VkDispatchIndirectCommand dispatchTemplate = drawGroup->Chunks.dispatchCmdTemplate;

        Vk::BufferUpdateInfo dispatchUpdateInfo{};
        dispatchUpdateInfo.buffer = drawGroup->Chunks.mortonIndirectDispatchBuffer.GetHandle(fIdx, isGpu);
        dispatchUpdateInfo.offset = 0;
        dispatchUpdateInfo.size = sizeof(VkDispatchIndirectCommand);
        dispatchUpdateInfo.pData = &dispatchTemplate;
        Vk::BufferUtils::UpdateBuffer(context.cmd, dispatchUpdateInfo);

        Vk::BufferBarrierInfo dispatchBarrier{};
        dispatchBarrier.buffer = dispatchUpdateInfo.buffer;
        dispatchBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        dispatchBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        dispatchBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        dispatchBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, dispatchBarrier);

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_staticCount, 32);
        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo chunkDataBarrier{};
        chunkDataBarrier.buffer = compManager->GetComponentBuffer(BufferNames::MortonChunkData, fIdx).buffer->Handle();
        chunkDataBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        chunkDataBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        chunkDataBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        chunkDataBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, chunkDataBarrier);

        Vk::BufferBarrierInfo chunkTransformIndicesBarrier{};
        chunkTransformIndicesBarrier.buffer = compManager->GetComponentBuffer(BufferNames::MortonChunkTransformsIndex, fIdx).buffer->Handle();
        chunkTransformIndicesBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        chunkTransformIndicesBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        chunkTransformIndicesBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        chunkTransformIndicesBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, chunkTransformIndicesBarrier);

        Vk::BufferBarrierInfo indirectDispatchBarrier{};
        indirectDispatchBarrier.buffer = drawGroup->Chunks.mortonIndirectDispatchBuffer.GetHandle(fIdx, isGpu);
        indirectDispatchBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectDispatchBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        indirectDispatchBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        indirectDispatchBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, indirectDispatchBarrier);

        Vk::BufferBarrierInfo indirectDrawBarrier{};
        indirectDrawBarrier.buffer = drawGroup->Chunks.mortonIndirectDrawBuffer.GetHandle(fIdx, isGpu);
        indirectDrawBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectDrawBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        indirectDrawBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        indirectDrawBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, indirectDrawBarrier);
    }
}