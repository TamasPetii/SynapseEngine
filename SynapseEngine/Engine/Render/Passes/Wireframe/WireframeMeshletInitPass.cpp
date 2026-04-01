#include "WireframeMeshletInitPass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneDrawData.h"
#include "Engine/Vk/Buffer/BufferUtils.h"

namespace Syn
{
    void WireframeMeshletInitPass::Transfer(const RenderContext& context)
    {
        auto scene = context.scene;
        uint32_t fIdx = context.frameIndex;

        auto drawData = scene->GetSceneDrawData();

        if (drawData->activeDescriptorCount == 0)
            return;

        Vk::BufferBarrierInfo instanceBarrier{};
        instanceBarrier.buffer = drawData->debugInstanceBuffers[fIdx]->Handle();
        instanceBarrier.srcStage = VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT;
        instanceBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        instanceBarrier.dstStage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        instanceBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, instanceBarrier);

        Vk::BufferBarrierInfo aabbCmdBarrier{};
        aabbCmdBarrier.buffer = drawData->debugAabbIndirectBuffers[fIdx]->Handle();
        aabbCmdBarrier.srcStage = VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT;
        aabbCmdBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        aabbCmdBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        aabbCmdBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, aabbCmdBarrier);

        Vk::BufferBarrierInfo sphereCmdBarrier{};
        sphereCmdBarrier.buffer = drawData->debugSphereIndirectBuffers[fIdx]->Handle();
        sphereCmdBarrier.srcStage = VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT;
        sphereCmdBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        sphereCmdBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        sphereCmdBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, sphereCmdBarrier);
    }
}