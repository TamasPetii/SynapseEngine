#include "DirectionLightShadowStaticChunkCullingPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/DirectionLightShadowCullingPC.glsl"

    bool DirectionLightShadowStaticChunkCullingPass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<DirectionLightComponent>();
        auto settings = context.scene->GetSettings();

        return settings->culling.directionLightShadowCullingDevice == CullingDeviceType::GPU 
            && settings->culling.directionLightShadowSpatialAcceleration == SpatialAccelerationType::StaticBvh 
            && pool && pool->Size() > 0;
    }

    void DirectionLightShadowStaticChunkCullingPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("DirectionLightShadowStaticChunkCullingProgram", {
            ShaderNames::DirectionLightShadowStaticChunkCullingComp
            }, config);
    }

    void DirectionLightShadowStaticChunkCullingPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();

        _activeChunkCount = drawData->Chunks.chunkCounter.load(std::memory_order_relaxed);
        _activeLights = static_cast<uint32_t>(scene->GetRegistry()->GetPool<DirectionLightComponent>()->Size());

        if (_activeChunkCount == 0 || _activeLights == 0) return;

        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<DirectionLightShadowCullingPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void DirectionLightShadowStaticChunkCullingPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();

        uint32_t prevFrameIndex = (context.frameIndex + context.framesInFlight - 1) % context.framesInFlight;
        auto depthPyramid = context.scene->GetSceneDrawData()->DirectionLightShadow.shadowDepthPyramid[prevFrameIndex].get();
        auto maxSampler = imageManager->GetSampler(SamplerNames::MaxReduction);

        Vk::PushDescriptorWriter pushWriter;
        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            maxSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        //pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void DirectionLightShadowStaticChunkCullingPass::Dispatch(const RenderContext& context) {
        if (_activeChunkCount == 0 || _activeLights == 0) return;

        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        //Todo: Direction Light Culling and indirect dispatch

        // 3D Grid Dispatch: X = Chunks, Y = Lights, Z = Cascades
        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(_activeChunkCount, ComputeGroupSize::Buffer32D);
        vkCmdDispatch(context.cmd, groupCountX, _activeLights, CASCADES_PER_LIGHT);

        VkBuffer dispatchBuf = drawData->DirectionLightShadow.staticChunkDispatchBuffer.GetHandle(fIdx);
        Vk::BufferBarrierInfo cullBarrier{};
        cullBarrier.buffer = dispatchBuf;
        cullBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        cullBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        cullBarrier.dstStage = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        cullBarrier.dstAccess = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, cullBarrier);

        Vk::BufferBarrierInfo chunkBarrier{};
        chunkBarrier.buffer = scene->GetComponentBufferManager()->GetComponentBuffer(BufferNames::DirectionLightShadowStaticChunkVisibleIndex, fIdx).buffer->Handle();
        chunkBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        chunkBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        chunkBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        chunkBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, chunkBarrier);
    }
}