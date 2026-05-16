#include "SceneAabbPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/ChunkBuilderPC.glsl"

    void SceneAabbPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("StaticSceneAABBProgram", { 
            ShaderNames::StaticSceneAABB
        }, config);
    }

    bool SceneAabbPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();
        return pool && !pool->GetStorage().GetStaticEntities().empty();
    }

    void SceneAabbPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        _staticCount = static_cast<uint32_t>(scene->GetRegistry()->GetPool<TransformComponent>()->GetStorage().GetStaticEntities().size());

        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGeometryGpuCulling;

        ChunkBuilderPC pc{};
        pc.frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx, isGpu);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(ChunkBuilderPC), &pc);
    }

    void SceneAabbPass::Dispatch(const RenderContext& context) {
        if (_staticCount == 0) return;

        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        auto settings = scene->GetSettings();
        uint32_t fIdx = context.frameIndex;
        bool isGpu = settings->enableGeometryGpuCulling;

        struct {
            uint32_t min[3] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
            uint32_t max[3] = { 0x00000000, 0x00000000, 0x00000000 };
        } resetData;

        VkBuffer aabbBufferHandle = drawData->Chunks.sceneAabbBuffer.GetHandle(fIdx, isGpu);

        Vk::BufferUpdateInfo resetInfo{};
        resetInfo.buffer = aabbBufferHandle;
        resetInfo.offset = 0;
        resetInfo.size = sizeof(resetData);
        resetInfo.pData = &resetData;
        Vk::BufferUtils::UpdateBuffer(context.cmd, resetInfo);

        Vk::BufferBarrierInfo resetBarrier{};
        resetBarrier.buffer = aabbBufferHandle;
        resetBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        resetBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        resetBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        resetBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, resetBarrier);

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_staticCount, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo aabbBarrier{};
        aabbBarrier.buffer = drawData->Models.computeCountBuffer.GetHandle(fIdx, isGpu);
        aabbBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        aabbBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        aabbBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        aabbBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, aabbBarrier);
    }
}