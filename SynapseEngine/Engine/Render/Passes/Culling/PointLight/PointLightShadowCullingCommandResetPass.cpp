#include "PointLightShadowCullingCommandResetPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Component/Light/Point/PointLightComponent.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/CullingCommandResetPC.glsl"

    bool PointLightShadowCullingCommandResetPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<PointLightComponent>();

        return context.scene->GetSettings()->culling.pointLightShadowCullingDevice == CullingDeviceType::GPU
               && pool && pool->Size() > 0;
    }

    void PointLightShadowCullingCommandResetPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("PointLightShadowCullingCommandResetProgram", {
            ShaderNames::PointLightShadowCullingCommandResetComp
            }, config);
    }

    void PointLightShadowCullingCommandResetPass::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        _totalCommands = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;

        Vk::PushConstant<CullingCommandResetPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void PointLightShadowCullingCommandResetPass::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        uint32_t dispatchCount = std::max(1u, ComputeGroupSize::CalculateDispatchCount(_totalCommands, ComputeGroupSize::Buffer256D));
        vkCmdDispatch(context.cmd, dispatchCount, 1, 1);

        Vk::BufferBarrierInfo indirectBarrier{};
        indirectBarrier.buffer = drawData->PointLightShadow.indirectBuffer.GetHandle(fIdx);
        indirectBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        indirectBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        indirectBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, indirectBarrier);

        VkBuffer modelOutputBuf = drawData->PointLightShadow.modelDispatchBuffer.GetHandle(fIdx);

        Vk::BufferUpdateInfo updateInfo{};
        updateInfo.buffer = modelOutputBuf;
        updateInfo.offset = 0;
        updateInfo.size = sizeof(VkDispatchIndirectCommand);
        updateInfo.pData = &drawData->PointLightShadow.dispatchCmdTemplate;
        Vk::BufferUtils::UpdateBuffer(context.cmd, updateInfo);

        Vk::BufferBarrierInfo updateBarrier{};
        updateBarrier.buffer = modelOutputBuf;
        updateBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        updateBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        updateBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        updateBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, updateBarrier);
    }
}