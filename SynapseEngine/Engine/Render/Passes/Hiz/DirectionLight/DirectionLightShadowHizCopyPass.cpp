#include "DirectionLightShadowHizCopyPass.h"
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
#include "Engine/Component/Light/Direction/DirectionLightComponent.h"
#include <glm/glm.hpp>
#include "Engine/Vk/Rendering/PushConstant.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/HizLinearizeDepthPC.glsl"

    bool DirectionLightShadowHizCopyPass::ShouldExecute(const RenderContext& context) const
    {
        auto pool = context.scene->GetRegistry()->GetPool<DirectionLightComponent>();
        return context.scene->GetSettings()->enableGeometryGpuCulling && pool && pool->Size() > 0;
    }

    void DirectionLightShadowHizCopyPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();
        _shaderProgram = shaderManager->CreateProgram("DirectionLightShadowHizCopyProgram", {
			ShaderNames::HizCopyComp
            });
    }

    void DirectionLightShadowHizCopyPass::PrepareFrame(const RenderContext& context) {
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        auto& shadowAtlas = drawData->DirectionLightShadow.shadowAtlas[fIdx];
        auto& depthPyramid = drawData->DirectionLightShadow.shadowDepthPyramid[fIdx];

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

    void DirectionLightShadowHizCopyPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();
        auto drawData = context.scene->GetSceneDrawData();
        uint32_t fIdx = context.frameIndex;

        auto& shadowAtlas = drawData->DirectionLightShadow.shadowAtlas[fIdx];
        auto& depthPyramid = drawData->DirectionLightShadow.shadowDepthPyramid[fIdx];

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

    void DirectionLightShadowHizCopyPass::PushConstants(const RenderContext& context) {
		auto scene = context.scene;
		auto fIdx = context.frameIndex;

        Vk::PushConstant<HizLinearizeDepthPC> pc;
        pc->frameGlobalContextBufferAddr = scene->GetSceneDrawData()->frameContextBuffer.GetAddress(fIdx, true);
        pc->outImageSize = glm::vec2(SHADOW_ATLAS_SIZE, SHADOW_ATLAS_SIZE);
        pc.Push(context.cmd, _shaderProgram->GetLayout());
    }

    void DirectionLightShadowHizCopyPass::Dispatch(const RenderContext& context) {
        uint32_t groupCountX = ComputeGroupSize::CalculateDispatchCount(SHADOW_ATLAS_SIZE, ComputeGroupSize::Image16D);
        uint32_t groupCountY = ComputeGroupSize::CalculateDispatchCount(SHADOW_ATLAS_SIZE, ComputeGroupSize::Image16D);

        vkCmdDispatch(context.cmd, groupCountX, groupCountY, 1);

        auto drawData = context.scene->GetSceneDrawData();
        auto& depthPyramid = drawData->DirectionLightShadow.shadowDepthPyramid[context.frameIndex];

        depthPyramid->TransitionLayout(
            context.cmd,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            VK_ACCESS_2_SHADER_READ_BIT
        );
    }
}