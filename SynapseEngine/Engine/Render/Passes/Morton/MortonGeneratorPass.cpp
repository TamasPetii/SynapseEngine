#include "MortonGeneratorPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Component/Core/TransformComponent.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/ChunkBuilderPC.glsl"

    void MortonGeneratorPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("MortonGeneratorProgram", { 
            ShaderNames::MortonGenerator 
        }, config);
    }

    bool MortonGeneratorPass::ShouldExecute(const RenderContext& context) const {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();

        bool isEnabled = context.scene->GetSettings()->culling.geometrySpatialAcceleration == SpatialAccelerationType::MortonBvh
            || context.scene->GetSettings()->culling.directionLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh
            || context.scene->GetSettings()->culling.pointLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh
            || context.scene->GetSettings()->culling.spotLightShadowSpatialAcceleration == SpatialAccelerationType::MortonBvh;

        if (!isEnabled || !pool || pool->GetStorage().GetStaticEntities().empty()) {
            _wasEnabled = false;
            return false;
        }

        if (!_wasEnabled) {
            _needsRebuild = true;
        }
        _wasEnabled = true;

        bool hasDirty = !pool->GetStorage().GetDirtyStatics().empty();
        return hasDirty || _needsRebuild || (_countdown > 0);
    }

    void MortonGeneratorPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        _staticCount = static_cast<uint32_t>(scene->GetRegistry()->GetPool<TransformComponent>()->GetStorage().GetStaticEntities().size());

        Vk::PushConstant<ChunkBuilderPC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(context.frameIndex);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void MortonGeneratorPass::Dispatch(const RenderContext& context) {
        auto pool = context.scene->GetRegistry()->GetPool<TransformComponent>();
        bool hasDirty = !pool->GetStorage().GetDirtyStatics().empty();

        if (hasDirty || _needsRebuild) {
            _countdown = context.framesInFlight;
            _needsRebuild = false;
        }

        if (_countdown > 0) {
            _countdown--;
        }

        if (_staticCount == 0) return;

        auto scene = context.scene;
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        VkBuffer keysHandle = compManager->GetComponentBuffer(BufferNames::MortonKeysData, fIdx).buffer->Handle();
        VkBuffer valuesHandle = compManager->GetComponentBuffer(BufferNames::MortonValuesData, fIdx).buffer->Handle();

        Vk::BufferFillInfo keysFill{};
        keysFill.buffer = keysHandle;
        keysFill.data = 0xFFFFFFFF;
        Vk::BufferUtils::FillBuffer(context.cmd, keysFill);

        Vk::BufferBarrierInfo keysFillBarrier{};
        keysFillBarrier.buffer = keysHandle;
        keysFillBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        keysFillBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        keysFillBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        keysFillBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, keysFillBarrier);

        Vk::BufferBarrierInfo valuesFillBarrier{};
        valuesFillBarrier.buffer = valuesHandle;
        valuesFillBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        valuesFillBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        valuesFillBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        valuesFillBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, valuesFillBarrier);

        Vk::BufferFillInfo valuesFill{};
        valuesFill.buffer = valuesHandle;
        valuesFill.data = 0xFFFFFFFF;
        Vk::BufferUtils::FillBuffer(context.cmd, valuesFill);

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_staticCount, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, 1, 1);

        Vk::BufferBarrierInfo keysBarrier{};
        keysBarrier.buffer = compManager->GetComponentBuffer(BufferNames::MortonKeysData, fIdx).buffer->Handle();
        keysBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        keysBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        keysBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        keysBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, keysBarrier);

        Vk::BufferBarrierInfo valuesBarrier{};
        valuesBarrier.buffer = compManager->GetComponentBuffer(BufferNames::MortonValuesData, fIdx).buffer->Handle();
        valuesBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        valuesBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        valuesBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        valuesBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, valuesBarrier);
    }
}