#include "SpotLightShadowAtlasAllocatorPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Component/Light/Spot/SpotLightShadowComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/SpotLightShadowCullingPC.glsl"

    void SpotLightShadowAtlasAllocatorPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        _shaderProgramId = shaderManager->LoadProgramAsync("SpotLightShadowAtlasAllocatorProgram", {
            ShaderNames::SpotLightShadowAtlasAllocatorComp
            }, { .useDescriptorBuffers = false });
    }

    bool SpotLightShadowAtlasAllocatorPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<SpotLightShadowComponent>();
        return context.scene->GetSettings()->culling.spotLightCullingDevice == CullingDeviceType::GPU
            && pool && pool->Size() > 0;
    }

    void SpotLightShadowAtlasAllocatorPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<SpotLightShadowCullingPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SpotLightShadowAtlasAllocatorPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        vkCmdDispatch(context.cmd, 1, 1, 1);

        Vk::BufferBarrierInfo shadowDataBarrier{};
        shadowDataBarrier.buffer = compManager->GetComponentBuffer(BufferNames::SpotLightShadowData, fIdx).buffer->Handle();
        shadowDataBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        shadowDataBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        shadowDataBarrier.dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        shadowDataBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, shadowDataBarrier);

        Vk::BufferBarrierInfo gridLookupBarrier{};
        gridLookupBarrier.buffer = scene->GetSceneDrawData()->SpotLightShadow.gridLookupBuffer.GetHandle(fIdx);
        gridLookupBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        gridLookupBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        gridLookupBarrier.dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        gridLookupBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, gridLookupBarrier);
    }
}