#include "ClusterSpotLightCountPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Buffer/BufferUtils.h"

namespace Syn 
{
    #include "Engine/Shaders/Includes/PushConstants/ClusterLightCountPC.glsl"

    void ClusterSpotLightCountPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("ClusterSpotLightCountProgram", {
            ShaderNames::ClusterSpotLightCount
            }, config);
    }

    void ClusterSpotLightCountPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        uint32_t fIdx = context.frameIndex;
        bool isGpu = scene->GetSettings()->enableGeometryGpuCulling;

        ClusterLightCountPC pc{};
		pc.frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx, true);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(ClusterLightCountPC), &pc);
    }

    void ClusterSpotLightCountPass::Dispatch(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();

        VkBuffer indirectBuffer = drawData->ForwardPlus.dispatchArgsBuffer.GetHandle(context.frameIndex, true);
        vkCmdDispatchIndirect(context.cmd, indirectBuffer, offsetof(ForwardPlusDispatchArgs, spotSlowCount));
    }
}