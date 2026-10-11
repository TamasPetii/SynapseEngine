#include "ClusterPointLightSinglePassHaradaPlus.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {
#include "Engine/Shaders/Includes/PushConstants/ClusterLightWritePC.glsl"

    void ClusterPointLightSinglePassHaradaPlus::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("ClusterPointLightSingleProgramHaradaPlus", {
            ShaderNames::ClusterPointLightSingleHaradaPlus
            }, config);
    }

    void ClusterPointLightSinglePassHaradaPlus::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<ClusterLightWritePC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void ClusterPointLightSinglePassHaradaPlus::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();

        VkBuffer indirectBuffer = drawData->ForwardPlus.dispatchArgsBuffer.GetHandle(context.frameIndex);
        vkCmdDispatchIndirect(context.cmd, indirectBuffer, offsetof(ForwardPlusDispatchArgs, pointFastPath));
    }
}