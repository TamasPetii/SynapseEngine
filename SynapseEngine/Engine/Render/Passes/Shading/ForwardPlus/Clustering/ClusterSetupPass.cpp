#include "ClusterSetupPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Component/Core/CameraComponent.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Scene/DrawData/ForwardPlusDrawGroup.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/ClusterSetupPC.glsl"

    void ClusterSetupPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram("ClusterSetupProgram", {
            ShaderNames::ClusterSetup
            }, config);
    }

    void ClusterSetupPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto drawData = scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        uint32_t fIdx = context.frameIndex;
        
        uint32_t cameraEntity = scene->GetSceneCameraEntity();
        const auto& camera = scene->GetRegistry()->GetComponent<CameraComponent>(cameraEntity);

        ClusterSetupPC pc{};
		pc.frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx, true);

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(ClusterSetupPC), &pc);
    }

    void ClusterSetupPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);

        //Using current frame's depth pyramid!
        auto depthPyramid = rtGroup->GetImage(RenderTargetNames::DepthPyramid);
        auto defaultSampler = imageManager->GetSampler(SamplerNames::NearestClampEdge);

        Vk::PushDescriptorWriter pushWriter;
        pushWriter.AddCombinedImageSampler(
            0,
            depthPyramid->GetView(Vk::ImageViewNames::Default),
            defaultSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void ClusterSetupPass::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        auto drawData = context.scene->GetSceneDrawData();
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        uint32_t width = rtGroup->GetWidth();
        uint32_t height = rtGroup->GetHeight();

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(width, drawData->ForwardPlus.tileSize);
        uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(height, drawData->ForwardPlus.tileSize);

        //Todo: Fill buffer count!

        vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

        Vk::BufferBarrierInfo tileGridBarrier{};
        tileGridBarrier.buffer = drawData->ForwardPlus.tileGridBuffer.GetHandle(fIdx, true);
        tileGridBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        tileGridBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        tileGridBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        tileGridBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, tileGridBarrier);

        Vk::BufferBarrierInfo clusterListBarrier{};
        clusterListBarrier.buffer = drawData->ForwardPlus.clusterListBuffer.GetHandle(fIdx, true);
        clusterListBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        clusterListBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        clusterListBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        clusterListBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, clusterListBarrier);

        Vk::BufferBarrierInfo countBarrier{};
        countBarrier.buffer = drawData->ForwardPlus.clusterCountBuffer.GetHandle(fIdx, true);
        countBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        countBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        countBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        countBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, countBarrier);
    }
}