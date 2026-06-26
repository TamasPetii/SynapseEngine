#include "SpotLightShadowCullingMemoryBarrierPass.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"

namespace Syn {
    void SpotLightShadowCullingMemoryBarrierPass::Execute(const RenderContext& context) {
        auto scene = context.scene;

        auto pool = scene->GetRegistry()->GetPool<SpotLightComponent>();
        if (scene->GetSettings()->culling.spotLightShadowCullingDevice != CullingDeviceType::GPU || !pool || pool->Size() == 0) {
            return;
        }

        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        Vk::BufferBarrierInfo sortKeysBarrier{};
        sortKeysBarrier.buffer = drawData->SpotLightShadow.drawCallKeyBuffer.GetHandle(fIdx);
        sortKeysBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        sortKeysBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        sortKeysBarrier.dstStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        sortKeysBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, sortKeysBarrier);

        Vk::BufferBarrierInfo instanceBarrier{};
        instanceBarrier.buffer = drawData->SpotLightShadow.instanceBuffer.GetHandle(fIdx);
        instanceBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        instanceBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        instanceBarrier.dstStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        instanceBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, instanceBarrier);

        Vk::BufferBarrierInfo countBarrier{};
        countBarrier.buffer = drawData->SpotLightShadow.visibleMeshCountDispatchBuffer.GetHandle(fIdx);
        countBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        countBarrier.dstStage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        countBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, countBarrier);

        Vk::BufferBarrierInfo indirectBarrier{};
        indirectBarrier.buffer = drawData->SpotLightShadow.indirectBuffer.GetHandle(fIdx);
        indirectBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        indirectBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        indirectBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, indirectBarrier);
    }
}