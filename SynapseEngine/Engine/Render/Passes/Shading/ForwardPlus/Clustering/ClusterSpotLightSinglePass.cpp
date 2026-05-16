#include "ClusterSpotLightSinglePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Render/RenderNames.h"

namespace Syn {
#include "Engine/Shaders/Includes/PushConstants/ClusterLightWritePC.glsl"

    void ClusterSpotLightSinglePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("ClusterSpotLightSingleProgram", {
            ShaderNames::ClusterSpotLightSingle
            }, config);
    }

    void ClusterSpotLightSinglePass::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        ClusterLightWritePC pc{};
        pc.frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx, true);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(ClusterLightWritePC), &pc);
    }

    void ClusterSpotLightSinglePass::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();

        VkBuffer indirectBuffer = drawData->ForwardPlus.dispatchArgsBuffer.GetHandle(context.frameIndex, true);
        vkCmdDispatchIndirect(context.cmd, indirectBuffer, offsetof(ForwardPlusDispatchArgs, spotFastPath));
    }
}