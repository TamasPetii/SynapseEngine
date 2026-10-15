#include "ClusterUberCullingPassDBC.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Render/ShaderNames.h"

namespace Syn 
{
    #include "Engine/Shaders/Includes/PushConstants/ClusterSetupPC.glsl"

    void ClusterUberCullingPassDBC::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgramId = shaderManager->LoadProgramAsync("ClusterUberProgramDBC", {
            ShaderNames::ClusterUberCullingDBC
            }, config);
    }

    void ClusterUberCullingPassDBC::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::Get<ImageManager>();
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);

        auto opaqueDepth = rtGroup->GetImage(RenderTargetNames::OpaqueDepth);
        auto defaultSampler = imageManager->GetSampler(SamplerNames::NearestClampEdge);

        Vk::PushDescriptorWriter pushWriter;
        pushWriter.AddCombinedImageSampler(
            0,
            opaqueDepth->GetView(),
            defaultSampler->Handle(),
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void ClusterUberCullingPassDBC::PushConstants(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        Vk::PushConstant<ClusterSetupPC> pc;
        pc->frameGlobalContextBufferAddr = drawData->frameContextBuffer.GetAddress(fIdx);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void ClusterUberCullingPassDBC::Dispatch(const RenderContext& context) {
        auto scene = context.scene;
        auto rtGroup = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Main, context.frameIndex);
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        uint32_t width = rtGroup->GetWidth();
        uint32_t height = rtGroup->GetHeight();

        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(width, scene->GetSettings()->lighting.tileSize);
        uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(height, scene->GetSettings()->lighting.tileSize);

        VkDispatchIndirectCommand resetCmd{ 0, 1, 1 };
        Vk::BufferUpdateInfo updateInfo{};
        updateInfo.buffer = drawData->ForwardPlus.clusterCountBuffer.GetHandle(fIdx);
        updateInfo.offset = 0;
        updateInfo.size = sizeof(VkDispatchIndirectCommand);
        updateInfo.pData = &resetCmd;
        Vk::BufferUtils::UpdateBuffer(context.cmd, updateInfo);

        Vk::BufferBarrierInfo updateBarrier{};
        updateBarrier.buffer = updateInfo.buffer;
        updateBarrier.srcStage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        updateBarrier.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        updateBarrier.dstStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        updateBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, updateBarrier);

        vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

        Vk::BufferBarrierInfo clusterBarrier{};
        clusterBarrier.buffer = drawData->ForwardPlus.clusterListBuffer.GetHandle(fIdx);
        clusterBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        clusterBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        clusterBarrier.dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        clusterBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, clusterBarrier);

        Vk::BufferBarrierInfo tileGridBarrier{};
        tileGridBarrier.buffer = drawData->ForwardPlus.tileGridBuffer.GetHandle(fIdx);
        tileGridBarrier.srcStage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        tileGridBarrier.srcAccess = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        tileGridBarrier.dstStage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        tileGridBarrier.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
        Vk::BufferUtils::InsertBarrier(context.cmd, tileGridBarrier);
    }
}