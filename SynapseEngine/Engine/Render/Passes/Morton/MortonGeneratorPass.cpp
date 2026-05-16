#include "MortonGeneratorPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/ChunkBuilderPC.glsl"

    void MortonGeneratorPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("MortonGeneratorProgram", { 
            ShaderNames::MortonGenerator 
        }, config);
    }

    bool MortonGeneratorPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();
        return pool && !pool->GetStorage().GetStaticEntities().empty();
    }

    void MortonGeneratorPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        _staticCount = static_cast<uint32_t>(scene->GetRegistry()->GetPool<TransformComponent>()->GetStorage().GetStaticEntities().size());

        ChunkBuilderPC pc{};
        pc.frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(context.frameIndex, scene->GetSettings()->enableGeometryGpuCulling);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(ChunkBuilderPC), &pc);
    }

    void MortonGeneratorPass::Dispatch(const RenderContext& context) {
        if (_staticCount == 0) return;

        auto scene = context.scene;
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_staticCount, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo keysBarrier{};
        keysBarrier.buffer = compManager->GetComponentBuffer(BufferNames::MortonKeysData, fIdx).buffer->Handle();
        keysBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        keysBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        keysBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        keysBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, keysBarrier);
    }
}