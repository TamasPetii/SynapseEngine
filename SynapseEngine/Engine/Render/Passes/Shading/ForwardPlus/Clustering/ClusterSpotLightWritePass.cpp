#include "ClusterSpotLightWritePass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Buffer/BufferUtils.h"

namespace Syn {
    #include "Engine/Shaders/Includes/PushConstants/ClusterLightWritePC.glsl"

    bool ClusterSpotLightWritePass::ShouldExecute(const RenderContext& context) const {
        return context.scene->GetSettings()->enableForwardPlus;
    }

    void ClusterSpotLightWritePass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("ClusterSpotLightWriteProgram", {
            ShaderNames::ClusterSpotLightWrite
            }, config);
    }

    void ClusterSpotLightWritePass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGpuCulling;

        ClusterLightWritePC pc{};
		pc.frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx, true);
        
        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(ClusterLightWritePC), &pc);
    }

    void ClusterSpotLightWritePass::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        vkCmdDispatch(context.cmd, drawData->ForwardPlus.maxClusters, 1, 1);
    }
}