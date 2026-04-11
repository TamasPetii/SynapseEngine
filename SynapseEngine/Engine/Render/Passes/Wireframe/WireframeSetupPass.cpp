#include "WireframeSetupPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/WireframeSetupPC.glsl"

    bool WireframeSetupPass::ShouldExecute(const RenderContext& context) const
    {
        return context.scene->GetSettings()->enableWireframeMeshSphere || context.scene->GetSettings()->enableWireframeMeshAabb;
    }

    void WireframeSetupPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("WireframeSetupProgram", {
            ShaderNames::WireframeSetup
            });
    }

    void WireframeSetupPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();

        uint32_t totalCommands = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;
        if (totalCommands == 0) {
            _shouldDispatch = false;
            return;
        }

        _shouldDispatch = true;
        uint32_t fIdx = context.frameIndex;
		auto isGpu = scene->GetSettings()->enableGpuCulling;

        WireframeSetupPC pc{};
        pc.globalIndirectCmdsAddr = drawData->Models.indirectBuffer.GetAddress(fIdx, isGpu);
        pc.aabbCmdsAddr = drawData->Debug.modelAabbIndirectBuffer.GetAddress(fIdx, isGpu);
        pc.sphereCmdsAddr = drawData->Debug.modelSphereIndirectBuffer.GetAddress(fIdx, isGpu);
        pc.totalTraditionalCommands = drawData->Models.activeTraditionalCount;
        pc.totalCommands = totalCommands;

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(WireframeSetupPC), &pc);
    }

    void WireframeSetupPass::Dispatch(const RenderContext& context) {
        if (!_shouldDispatch) return;

        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;
		auto isGpu = context.scene->GetSettings()->enableGpuCulling;

        uint32_t totalCommands = drawData->Models.activeTraditionalCount + drawData->Models.activeMeshletCount;
        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(totalCommands, ComputeGroupSize::Buffer256D);

        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo aabbBarrier{};
        aabbBarrier.buffer = drawData->Debug.modelAabbIndirectBuffer.GetHandle(fIdx, isGpu);
        aabbBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        aabbBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        aabbBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        aabbBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, aabbBarrier);

        Vk::BufferBarrierInfo sphereBarrier{};
        sphereBarrier.buffer = drawData->Debug.modelSphereIndirectBuffer.GetHandle(fIdx, isGpu);
        sphereBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sphereBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        sphereBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        sphereBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, sphereBarrier);
    }
}