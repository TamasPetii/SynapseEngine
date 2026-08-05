#include "SpotLightShadowFinalizeSetupPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Component/Light/Spot/SpotLightShadowComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/SpotLightShadowCullingPC.glsl"

    void SpotLightShadowFinalizeSetupPass::Initialize() {
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        _shaderProgramId = shaderManager->LoadProgramAsync("SpotLightShadowFinalizeSetupProgram", {
            ShaderNames::SpotLightShadowFinalizeSetupComp
            }, config);
    }

    bool SpotLightShadowFinalizeSetupPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<SpotLightShadowComponent>();
        return context.scene->GetSettings()->culling.spotLightShadowCullingDevice == CullingDeviceType::GPU
            && pool && pool->Size() > 0;
    }

    void SpotLightShadowFinalizeSetupPass::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<SpotLightShadowCullingPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SpotLightShadowFinalizeSetupPass::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        vkCmdDispatch(context.cmd, 1, 1, 1);

        Vk::BufferBarrierInfo setupBarrier{};
        setupBarrier.buffer = drawData->SpotLightShadow.finalizeDispatchBuffer.GetHandle(fIdx);
        setupBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        setupBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        setupBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        setupBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, setupBarrier);
    }
}