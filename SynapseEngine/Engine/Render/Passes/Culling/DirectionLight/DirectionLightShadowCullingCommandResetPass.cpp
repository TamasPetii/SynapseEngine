#include "DirectionLightShadowCullingCommandResetPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Component/Light/Direction/DirectionLightShadowComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/CullingCommandResetPC.glsl"

    bool DirectionLightShadowCullingCommandResetPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<DirectionLightShadowComponent>();
        return context.scene->GetSettings()->culling.directionLightShadowCullingDevice == CullingDeviceType::GPU 
            && pool && pool->Size() > 0;
    }

    void DirectionLightShadowCullingCommandResetPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("DirectionLightShadowCullingCommandResetProgram", {
            ShaderNames::DirectionLightShadowCullingCommandResetComp
            }, config);
    }

    void DirectionLightShadowCullingCommandResetPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = context.scene->GetSceneDrawData();

        uint32_t fIdx = context.frameIndex;

        _totalCommands = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;

        Vk::PushConstant<CullingCommandResetPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void DirectionLightShadowCullingCommandResetPass::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();

        uint32_t fIdx = context.frameIndex;

        uint32_t dispatchCount = std::max(1u, ComputeGroupSize::CalculateDispatchCount(_totalCommands, ComputeGroupSize::Buffer256D));
        vkCmdDispatch(context.cmd, dispatchCount, 1, 1);

        Vk::BufferBarrierInfo indirectBarrier{};
        indirectBarrier.buffer = drawData->DirectionLightShadow.indirectBuffer.GetHandle(fIdx);
        indirectBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        indirectBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        indirectBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, indirectBarrier);

        VkBuffer modelBuf = drawData->DirectionLightShadow.modelDispatchBuffer.GetHandle(fIdx);
        VkBuffer staticChunkBuf = drawData->DirectionLightShadow.staticChunkDispatchBuffer.GetHandle(fIdx);
        VkBuffer mortonChunkBuf = drawData->DirectionLightShadow.mortonChunkDispatchBuffer.GetHandle(fIdx);

        std::vector<Vk::BufferUpdateInfo> updates = {
            { modelBuf, 0, sizeof(VkDispatchIndirectCommand), &drawData->DirectionLightShadow.dispatchCmdTemplate },
            { staticChunkBuf, 0, sizeof(VkDispatchIndirectCommand), &drawData->DirectionLightShadow.dispatchCmdTemplate },
            { mortonChunkBuf, 0, sizeof(VkDispatchIndirectCommand), &drawData->DirectionLightShadow.dispatchCmdTemplate }
        };

        for (const auto& updateInfo : updates) {
            Vk::BufferUtils::UpdateBuffer(context.cmd, updateInfo);

            Vk::BufferBarrierInfo updateBarrier{};
            updateBarrier.buffer = updateInfo.buffer;
            updateBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            updateBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            updateBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            updateBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
            Vk::BufferUtils::InsertBarrier(context.cmd, updateBarrier);
        }
    }
}