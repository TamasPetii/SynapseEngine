#include "SpotLightShadowHizCopyPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Image/ImageUtils.h" 
#include "Engine/Render/ComputeGroupSize.h"
#include "Engine/Component/Light/Spot/SpotLightComponent.h"
#include <glm/glm.hpp>
#include "Engine/Vk/Rendering/PushConstant.h"
#include "Engine/Scene/DrawData/SpotLightShadowDrawGroup.h"

namespace Syn {

#include "Engine/Shaders/Includes/PushConstants/HizLinearizeDepthPC.glsl"

    bool SpotLightShadowHizCopyPass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<SpotLightComponent>();
        return pool && pool->Size() > 0;
    }

    void SpotLightShadowHizCopyPass::Initialize() {
        auto shaderManager = ServiceLocator::Get<ShaderManager>();
        _shaderProgram = shaderManager->CreateProgram("SpotLightShadowHizCopyProgram", {
            ShaderNames::SpotHizLinearizeSingleDepth
            });
    }

    void SpotLightShadowHizCopyPass::PrepareFrame(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        auto& shadowAtlas = drawData->SpotLightShadow.shadowAtlas[fIdx];
        auto& depthPyramid = drawData->SpotLightShadow.shadowDepthPyramid[fIdx];

        _imageTransitions.push_back({
            shadowAtlas.get(),
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            false
            });

        _imageTransitions.push_back({
            depthPyramid.get(),
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT,
            true
            });
    }

    void SpotLightShadowHizCopyPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::Get<ImageManager>();
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        auto& shadowAtlas = drawData->SpotLightShadow.shadowAtlas[fIdx];
        auto& depthPyramid = drawData->SpotLightShadow.shadowDepthPyramid[fIdx];

        auto sampler = imageManager->GetSampler(SamplerNames::NearestClampEdge);

        Vk::PushDescriptorWriter pushWriter;

        pushWriter.AddCombinedImageSampler(
            0,
            shadowAtlas->GetView(Vk::ImageViewNames::Default),
            sampler->Handle(),
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        );

        std::string mip0ViewName = std::string(Vk::ImageViewNames::Default) + std::string(Vk::ImageViewNames::Mip) + "0";

        pushWriter.AddStorageImage(
            1,
            depthPyramid->GetView(mip0ViewName),
            VK_IMAGE_LAYOUT_GENERAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_COMPUTE);
    }

    void SpotLightShadowHizCopyPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto fIdx = context.frameIndex;

        Vk::PushConstant<HizLinearizeDepthPC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx);
        pc->outImageSize = glm::vec2(SPOT_SHADOW_ATLAS_SIZE, SPOT_SHADOW_ATLAS_SIZE);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void SpotLightShadowHizCopyPass::Dispatch(const RenderContext& context) {
        constexpr uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(SPOT_SHADOW_ATLAS_SIZE, ComputeGroupSize::Image16D);
        constexpr uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(SPOT_SHADOW_ATLAS_SIZE, ComputeGroupSize::Image16D);

        vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

        auto drawData = context.scene->GetSceneDrawData();
        auto& depthPyramid = drawData->SpotLightShadow.shadowDepthPyramid[context.frameIndex];

        depthPyramid->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            VK_ACCESS_2_SHADER_READ_BIT
        );

        auto& shadowAtlas = drawData->SpotLightShadow.shadowAtlas[context.frameIndex];
        shadowAtlas->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_TASK_SHADER_BIT_EXT,
            VK_ACCESS_2_SHADER_READ_BIT
        );
    }
}