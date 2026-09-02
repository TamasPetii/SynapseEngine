// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "RendererFactory.h"
#include "Engine/Vk/Image/ImageConfig.h"

#include "Engine/Render/Pipelines/Setup/SetupAndInitPipeline.h"
#include "Engine/Render/Pipelines/Morton/MortonBvhBuilderPipeline.h"
#include "Engine/Render/Pipelines/Culling/MainCameraGeometryCullingPipeline.h"

#include "Engine/Render/Pipelines/Shadow/DirectionLight/Static/DirectionLightStaticShadowPipeline.h"
#include "Engine/Render/Pipelines/Shadow/DirectionLight/Dynamic/DirectionLightDynamicShadowPipeline.h"
#include "Engine/Render/Pipelines/Shadow/SpotLight/SpotLightShadowPipeline.h"
#include "Engine/Render/Pipelines/Shadow/PointLight/PointLightShadowPipeline.h"

#include "Engine/Render/Pipelines/Shading/ForwardPlus/ForwardPlusOpaqueDepthPipeline.h"
#include "Engine/Render/Pipelines/Shading/ForwardPlus/ForwardPlusTransparentDepthPipeline.h"
#include "Engine/Render/Pipelines/Shading/Deferred/DeferredGBufferPipeline.h"
#include "Engine/Render/Pipelines/Hiz/DepthPyramidHizPipeline.h"

#include "Engine/Render/Pipelines/PostProcess/Ssao/SsaoPipeline.h"
#include "Engine/Render/Pipelines/Shading/Deferred/DeferredLightingPipeline.h"
#include "Engine/Render/Pipelines/Shading/ForwardPlus/ForwardPlusClusteringPipeline.h"
#include "Engine/Render/Pipelines/Shading/ForwardPlus/ForwardPlusOpaqueLightingPipeline.h"

#include "Engine/Render/Pipelines/PostProcess/Environment/EnvironmentPipeline.h"
#include "Engine/Render/Pipelines/Wireframe/WireframePipeline.h"
#include "Engine/Render/Pipelines/Billboard/BillboardPipeline.h"
#include "Engine/Render/Pipelines/Shading/Wboit/TransparentWboitPipeline.h"
#include "Engine/Render/Pipelines/PostProcess/Bloom/BloomPipeline.h"
#include "Engine/Render/Pipelines/Preview/EditorPreviewPipeline.h"
#include "Engine/Render/Pipelines/Present/PresentationPipeline.h"

#include "Engine/Render/Passes/PostProcess/Outline/SelectionOutlinePass.h"
#include "Engine/Render/Passes/Shading/Visibility/DebugVisibilityPass.h"
#include "Engine/Render/Passes/Shadow/ShadowFinalTransitionPass.h"

#include "Engine/Vk/Image/ImageViewNames.h"
#include "RenderNames.h"

namespace Syn 
{
    std::unique_ptr<RenderManager> RendererFactory::CreateSceneRenderer(uint32_t framesInFlight) {
        auto renderManager = std::make_unique<RenderManager>(framesInFlight);
        auto rtManager = renderManager->GetRenderTargetManager();

        auto pipeline = std::make_unique<RenderPipeline>("ScenePipeline");

        // Setup & Culling
        pipeline->AddPass(std::make_unique<SetupAndInitPipeline>());
        pipeline->AddPass(std::make_unique<MortonBvhBuilderPipeline>());
        pipeline->AddPass(std::make_unique<MainCameraGeometryCullingPipeline>());

        // Shadows (Static & Dynamic)
        pipeline->AddPass(std::make_unique<DirectionLightStaticShadowPipeline>());
        pipeline->AddPass(std::make_unique<DirectionLightDynamicShadowPipeline>());
        pipeline->AddPass(std::make_unique<SpotLightShadowPipeline>());
        pipeline->AddPass(std::make_unique<PointLightShadowPipeline>());
        pipeline->AddPass(std::make_unique<ShadowFinalTransitionPass>());

        // Opaque Depth Prepasses
        pipeline->AddPass(std::make_unique<ForwardPlusOpaqueDepthPipeline>());
        pipeline->AddPass(std::make_unique<DeferredGBufferPipeline>());

        // Transparent Depth & Hi-Z
        pipeline->AddPass(std::make_unique<ForwardPlusTransparentDepthPipeline>());
        pipeline->AddPass(std::make_unique<DepthPyramidHizPipeline>());

        // SSAO
        pipeline->AddPass(std::make_unique<SsaoPipeline>());

        // Opaque Lighting
        pipeline->AddPass(std::make_unique<DeferredLightingPipeline>());
        pipeline->AddPass(std::make_unique<ForwardPlusClusteringPipeline>());
        pipeline->AddPass(std::make_unique<ForwardPlusOpaqueLightingPipeline>());

        // Environment, Wireframe & Editor
        pipeline->AddPass(std::make_unique<EnvironmentPipeline>());
        pipeline->AddPass(std::make_unique<WireframePipeline>());
        pipeline->AddPass(std::make_unique<BillboardPipeline>());

        // Transparent Shading (WBOIT)
        pipeline->AddPass(std::make_unique<TransparentWboitPipeline>());

        // Post-Process & Debug 
        pipeline->AddPass(std::make_unique<BloomPipeline>());
        pipeline->AddPass(std::make_unique<SelectionOutlinePass>());
        pipeline->AddPass(std::make_unique<DebugVisibilityPass>());
        pipeline->AddPass(std::make_unique<EditorPreviewPipeline>());

        // Present
        pipeline->AddPass(std::make_unique<PresentationPipeline>());

        pipeline->Initialize();
        renderManager->RegisterPipeline(RenderPipelineNames::ScenePipeline, std::move(pipeline));

        rtManager->CreateGroup(RenderTargetGroupNames::Main);

        uint32_t initWidth = 4;
        uint32_t initHeight = 4;

        uint32_t msaaSamples = 4;
        VkSampleCountFlagBits sampleCount = static_cast<VkSampleCountFlagBits>(msaaSamples);

        Vk::ImageConfig mainImageSpec{};
        mainImageSpec.width = initWidth;
        mainImageSpec.height = initHeight;
        mainImageSpec.type = VK_IMAGE_TYPE_2D;
        mainImageSpec.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        mainImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        mainImageSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::Main, mainImageSpec);

        Vk::ImageConfig mainMsaaSpec = mainImageSpec;
        mainMsaaSpec.samples = sampleCount;
        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::MainMSAA, mainMsaaSpec);

        Vk::ImageConfig colorImageSpec{};
        colorImageSpec.width = initWidth;
        colorImageSpec.height = initHeight;
        colorImageSpec.type = VK_IMAGE_TYPE_2D;
        colorImageSpec.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        colorImageSpec.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        colorImageSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        colorImageSpec.AddView(RenderTargetViewNames::Color, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .mipLevelCount = 1,
            .swizzle = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_ONE }
            });
        colorImageSpec.AddView(RenderTargetViewNames::Metallic, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .mipLevelCount = 1,
            .swizzle = { VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_ONE }
            });

        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::ColorMetallic, colorImageSpec);

        Vk::ImageConfig normalImageSpec{};
        normalImageSpec.width = initWidth;
        normalImageSpec.height = initHeight;
        normalImageSpec.type = VK_IMAGE_TYPE_2D;
        normalImageSpec.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        normalImageSpec.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        normalImageSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        normalImageSpec.AddView(RenderTargetViewNames::Normal, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .mipLevelCount = 1,
            .swizzle = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_ONE }
            });

        normalImageSpec.AddView(RenderTargetViewNames::Roughness, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .mipLevelCount = 1,
            .swizzle = { VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_ONE }
            });

        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::NormalRoughness, normalImageSpec);

        Vk::ImageConfig emissiveAoImageSpec{};
        emissiveAoImageSpec.width = initWidth;
        emissiveAoImageSpec.height = initHeight;
        emissiveAoImageSpec.type = VK_IMAGE_TYPE_2D;
        emissiveAoImageSpec.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        emissiveAoImageSpec.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        emissiveAoImageSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        emissiveAoImageSpec.AddView(RenderTargetViewNames::Emissive, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .mipLevelCount = 1,
            .swizzle = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_ONE }
            });

        emissiveAoImageSpec.AddView(RenderTargetViewNames::AmbientOcclusion, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .mipLevelCount = 1,
            .swizzle = { VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_ONE }
            });

        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::EmissiveAo, emissiveAoImageSpec);

        Vk::ImageConfig transparentAccumSpec{};
        transparentAccumSpec.width = initWidth;
        transparentAccumSpec.height = initHeight;
        transparentAccumSpec.type = VK_IMAGE_TYPE_2D;
        transparentAccumSpec.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        transparentAccumSpec.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        transparentAccumSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::TransparentAccum, transparentAccumSpec);

        Vk::ImageConfig transparentAccumMsaaSpec = transparentAccumSpec;
        transparentAccumMsaaSpec.samples = sampleCount;
        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::TransparentAccumMSAA, transparentAccumMsaaSpec);

        Vk::ImageConfig transparentRevealSpec{};
        transparentRevealSpec.width = initWidth;
        transparentRevealSpec.height = initHeight;
        transparentRevealSpec.type = VK_IMAGE_TYPE_2D;
        transparentRevealSpec.format = VK_FORMAT_R8_UNORM; 
        transparentRevealSpec.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        transparentRevealSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::TransparentReveal, transparentRevealSpec);

        Vk::ImageConfig transparentRevealMsaaSpec = transparentRevealSpec;
        transparentRevealMsaaSpec.samples = sampleCount;
        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::TransparentRevealMSAA, transparentRevealMsaaSpec);

        Vk::ImageConfig entityImageSpec{};
        entityImageSpec.width = initWidth;
        entityImageSpec.height = initHeight;
        entityImageSpec.type = VK_IMAGE_TYPE_2D;
        entityImageSpec.format = VK_FORMAT_R32G32_UINT;
        entityImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        entityImageSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::EntityIndex, entityImageSpec);

        Vk::ImageConfig entityImageMsaaSpec = entityImageSpec;
        entityImageMsaaSpec.samples = sampleCount;
        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::EntityIndexMSAA, entityImageMsaaSpec);

        Vk::ImageConfig depthPyramidImageSpec{};
        depthPyramidImageSpec.width = initWidth;
        depthPyramidImageSpec.height = initHeight;
        depthPyramidImageSpec.type = VK_IMAGE_TYPE_2D;
        depthPyramidImageSpec.format = VK_FORMAT_R32G32_SFLOAT;
        depthPyramidImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        depthPyramidImageSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        depthPyramidImageSpec.generateMipMaps = true;

        depthPyramidImageSpec.AddView(Vk::ImageViewNames::Default, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .perMipViews = true
            });

        depthPyramidImageSpec.AddView(RenderTargetViewNames::DepthOpaqueMax, Vk::ImageViewConfig{
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .swizzle = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ONE },
                    .perMipViews = true
            });

        depthPyramidImageSpec.AddView(RenderTargetViewNames::DepthTransparentMin, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .swizzle = { VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ONE },
            .perMipViews = true
            });

        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::DepthPyramid, depthPyramidImageSpec);

        Vk::ImageConfig bloomImageSpec{};
        bloomImageSpec.width = initWidth;
        bloomImageSpec.height = initHeight;
        bloomImageSpec.type = VK_IMAGE_TYPE_2D;
        bloomImageSpec.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        bloomImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        bloomImageSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        bloomImageSpec.generateMipMaps = true;

        bloomImageSpec.AddView(Vk::ImageViewNames::Default, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .perMipViews = true
            });

        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::Bloom, bloomImageSpec);

        Vk::ImageConfig opaqueDepthSpec{};
        opaqueDepthSpec.width = initWidth;
        opaqueDepthSpec.height = initHeight;
        opaqueDepthSpec.type = VK_IMAGE_TYPE_2D;
        opaqueDepthSpec.format = VK_FORMAT_D32_SFLOAT;
        opaqueDepthSpec.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        opaqueDepthSpec.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::OpaqueDepth, opaqueDepthSpec);

        Vk::ImageConfig opaqueDepthMsaaSpec = opaqueDepthSpec;
        opaqueDepthMsaaSpec.samples = sampleCount;
        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::OpaqueDepthMSAA, opaqueDepthMsaaSpec);

        Vk::ImageConfig transparentDepthSpec{};
        transparentDepthSpec.width = initWidth;
        transparentDepthSpec.height = initHeight;
        transparentDepthSpec.type = VK_IMAGE_TYPE_2D;
        transparentDepthSpec.format = VK_FORMAT_D32_SFLOAT;
        transparentDepthSpec.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        transparentDepthSpec.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::TransparentDepth, transparentDepthSpec);

        Vk::ImageConfig volumetricAoImageSpec{};
        volumetricAoImageSpec.width = initWidth;
        volumetricAoImageSpec.height = initHeight;
        volumetricAoImageSpec.type = VK_IMAGE_TYPE_2D;
        volumetricAoImageSpec.format = VK_FORMAT_R16_SFLOAT;
        volumetricAoImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        volumetricAoImageSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::SsaoAo, volumetricAoImageSpec);

        Vk::ImageConfig volumetricAoIntermediateImageSpec{};
        volumetricAoIntermediateImageSpec.width = initWidth;
        volumetricAoIntermediateImageSpec.height = initHeight;
        volumetricAoIntermediateImageSpec.type = VK_IMAGE_TYPE_2D;
        volumetricAoIntermediateImageSpec.format = VK_FORMAT_R16_SFLOAT;
        volumetricAoIntermediateImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        volumetricAoIntermediateImageSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Main, RenderTargetNames::SsaoAoIntermediate, volumetricAoIntermediateImageSpec);

        return renderManager;
    }
}