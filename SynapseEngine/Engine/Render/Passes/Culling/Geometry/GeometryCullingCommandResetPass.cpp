#include "GeometryCullingCommandResetPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/CullingCommandResetPC.glsl"

    bool GeometryCullingCommandResetPass::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->enableGeometryGpuCulling;
    }

    void GeometryCullingCommandResetPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("GeometryCullingCommandResetProgram", {
            ShaderNames::GeometryCullingCommandResetComp
            }, config);
    }

    void GeometryCullingCommandResetPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = context.scene->GetSceneDrawData();

        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGeometryGpuCulling;

        _totalCommands = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;

        Vk::PushConstant<CullingCommandResetPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx, isGpu);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void GeometryCullingCommandResetPass::Dispatch(const RenderContext& context) {
		auto drawData = context.scene->GetSceneDrawData();
        bool isGpu = context.scene->GetSettings()->enableGeometryGpuCulling;
        uint32_t fIdx = context.frameIndex;

        uint32_t dispatchCount = std::max(1u, ComputeGroupSize::CalculateDispatchCount(_totalCommands, ComputeGroupSize::Buffer256D));
        vkCmdDispatch(context.cmd, dispatchCount, 1, 1);

        Vk::BufferBarrierInfo indirectBarrier{};
        indirectBarrier.buffer = drawData->Models.indirectBuffer.GetHandle(fIdx, isGpu);
        indirectBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        indirectBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        indirectBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, indirectBarrier);

        { //Model->Mesh indirect command reset
            VkBuffer countBuf = drawData->Models.computeCountBuffer.GetHandle(fIdx, isGpu);
            Vk::BufferUtils::UpdateBuffer(context.cmd, {
                .buffer = countBuf,
                .offset = 0,
                .size = sizeof(VkDispatchIndirectCommand),
                .pData = &drawData->Models.dispatchCmdTemplate
                });

            Vk::BufferBarrierInfo updateBarrier{};
            updateBarrier.buffer = countBuf;
            updateBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            updateBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            updateBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            updateBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
            Vk::BufferUtils::InsertBarrier(context.cmd, updateBarrier);

        }

        { //Chunk->model indirect command reset
            VkBuffer dispatchBuf = drawData->Chunks.chunkIndirectDispatchBuffer.GetHandle(fIdx, isGpu);

            Vk::BufferUtils::UpdateBuffer(context.cmd, {
                .buffer = dispatchBuf,
                .offset = 0,
                .size = sizeof(VkDispatchIndirectCommand),
                .pData = &drawData->Chunks.dispatchCmdTemplate
                });

            Vk::BufferBarrierInfo updateBarrier{};
            updateBarrier.buffer = dispatchBuf;
            updateBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            updateBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            updateBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            updateBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
            Vk::BufferUtils::InsertBarrier(context.cmd, updateBarrier);
        }
    }
}