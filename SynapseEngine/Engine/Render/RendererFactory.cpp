#include "RendererFactory.h"
#include "Engine/Vk/Image/ImageConfig.h"

#include "Engine/Render/Passes/Billboard/BillboardTransitionPass.h"
#include "Engine/Render/Passes/Billboard/CameraBillboardPass.h"
#include "Engine/Render/Passes/Billboard/DirectionLightBillboardPass.h"
#include "Engine/Render/Passes/Billboard/PointLightBillboardPass.h"
#include "Engine/Render/Passes/Billboard/SpotLightBillboardPass.h"

#include "Engine/Render/Passes/Bloom/BloomPrefilterPass.h"
#include "Engine/Render/Passes/Bloom/BloomUpsamplePass.h"
#include "Engine/Render/Passes/Bloom/BloomDownsamplePass.h"
#include "Engine/Render/Passes/Bloom/BloomCompositePass.h"

#include "Engine/Render/Passes/Culling/ModelCullingPass.h"
#include "Engine/Render/Passes/Culling/MeshCullingPass.h"
#include "Engine/Render/Passes/Culling/PointLightCullingPass.h"
#include "Engine/Render/Passes/Culling/SpotLightCullingPass.h"
#include "Engine/Render/Passes/Culling/CullingCommandResetPass.h"

#include "Engine/Render/Passes/Setup/HizInitPass.h"
#include "Engine/Render/Passes/Hiz/HizLinearPreparePass.h"
#include "Engine/Render/Passes/Hiz/HizDownsamplePass.h"

#include "Engine/Render/Passes/Present/GuiPass.h"
#include "Engine/Render/Passes/Present/CompositePass.h"
#include "Engine/Render/Passes/Present/PresentationTransitionPass.h"

#include "Engine/Render/Passes/Setup/GLobalFrameSetupPass.h"

#include "Engine/Render/Passes/Shading/Common/DepthCopyPass.h"
#include "Engine/Render/Passes/Shading/Common/OpaqueInitPass.h"
#include "Engine/Render/Passes/Shading/Common/TransparentInitPass.h"

#include "Engine/Render/Passes/Shading/Deferred/GBuffer/OpaqueDeferredTransitionPass.h"
#include "Engine/Render/Passes/Shading/Deferred/GBuffer/MeshletOpaqueDeferredPass.h"
#include "Engine/Render/Passes/Shading/Deferred/GBuffer/TraditionalOpaqueDeferredPass.h"

#include "Engine/Render/Passes/Shading/Deferred/Lighting/DeferredLightTransitionPass.h"
#include "Engine/Render/Passes/Shading/Deferred/Lighting/DeferredEmissiveAoPass.h"
#include "Engine/Render/Passes/Shading/Deferred/Lighting/DeferredPointLightPass.h"
#include "Engine/Render/Passes/Shading/Deferred/Lighting/DeferredSpotLightPass.h"
#include "Engine/Render/Passes/Shading/Deferred/Lighting/DeferredDirectionLightPass.h"

#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterSetupPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterPointLightCountPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterSpotLightCountPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterPrefixSumPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterPointLightWritePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterSpotLightWritePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterLightWriteSyncPass.h"

#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/OpaqueDepthTransitionPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/MeshletOpaqueDepthPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/TraditionalOpaqueDepthPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/TransparentDepthTransitionPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/MeshletTransparentDepthPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/TraditionalTransparentDepthPrepass.h"

#include "Engine/Render/Passes/Shading/ForwardPlus/Lighting/OpaqueForwardTransitionPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Lighting/MeshletOpaqueForwardPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Lighting/TraditionalOpaqueForwardPass.h"

#include "Engine/Render/Passes/Shading/Wboit/TransparentForwardTransitionPass.h"
#include "Engine/Render/Passes/Shading/Wboit/MeshletTransparentForwardPass.h"
#include "Engine/Render/Passes/Shading/Wboit/TraditionalTransparentForwardPass.h"
#include "Engine/Render/Passes/Shading/Wboit/TransparentCompositeTransitionPass.h"
#include "Engine/Render/Passes/Shading/Wboit/TransparentCompositePass.h"

#include "Engine/Render/Passes/Wireframe/WireframeMeshSetupPass.h"
#include "Engine/Render/Passes/Wireframe/WireframeMeshAabbPass.h"
#include "Engine/Render/Passes/Wireframe/WireframeMeshSpherePass.h"
#include "Engine/Render/Passes/Wireframe/PointLightAabbWireframePass.h"
#include "Engine/Render/Passes/Wireframe/PointLightSphereWireframePass.h"
#include "Engine/Render/Passes/Wireframe/SpotLightAabbWireframePass.h"
#include "Engine/Render/Passes/Wireframe/SpotLightSphereWireframePass.h"

#include "Engine/Render/Passes/Shading/Visibility/DebugVisibilityPass.h"

#include "Engine/Vk/Image/ImageViewNames.h"
#include "RenderNames.h"

namespace Syn 
{
    std::unique_ptr<RenderManager> RendererFactory::CreateDeferredRenderer(uint32_t framesInFlight) {
        auto renderManager = std::make_unique<RenderManager>(framesInFlight);
        auto rtManager = renderManager->GetRenderTargetManager();

        auto pipeline = std::make_unique<RenderPipeline>();

		//Setup Passes
		pipeline->AddPass(std::make_unique<GlobalFrameSetupPass>());
		pipeline->AddPass(std::make_unique<OpaqueInitPass>());
		pipeline->AddPass(std::make_unique<TransparentInitPass>());
		pipeline->AddPass(std::make_unique<HizInitPass>());

		//Geometry Culling Passes
		pipeline->AddPass(std::make_unique<CullingCommandResetPass>());
        pipeline->AddPass(std::make_unique<ModelCullingPass>());
        pipeline->AddPass(std::make_unique<MeshCullingPass>());

		//Forward+ Depth Opaque Prepasses
		pipeline->AddPass(std::make_unique<OpaqueDepthTransitionPrepass>());
		pipeline->AddPass(std::make_unique<MeshletOpaqueDepthPrepass>(MaterialRenderType::Opaque1Sided));
		pipeline->AddPass(std::make_unique<MeshletOpaqueDepthPrepass>(MaterialRenderType::Opaque2Sided));
		pipeline->AddPass(std::make_unique<TraditionalOpaqueDepthPrepass>(MaterialRenderType::Opaque1Sided));
		pipeline->AddPass(std::make_unique<TraditionalOpaqueDepthPrepass>(MaterialRenderType::Opaque2Sided));

        //Deferred Depth Opaque Prepasses
		pipeline->AddPass(std::make_unique<OpaqueDeferredTransitionPass>());
		pipeline->AddPass(std::make_unique<MeshletOpaqueDeferredPass>(MaterialRenderType::Opaque1Sided));
        pipeline->AddPass(std::make_unique<MeshletOpaqueDeferredPass>(MaterialRenderType::Opaque2Sided));
		pipeline->AddPass(std::make_unique<TraditionalOpaqueDeferredPass>(MaterialRenderType::Opaque1Sided));
		pipeline->AddPass(std::make_unique<TraditionalOpaqueDeferredPass>(MaterialRenderType::Opaque2Sided));

        //Copy Opaque Depth to Transparent Depth Buffer
		pipeline->AddPass(std::make_unique<DepthCopyPass>());

        //Forward+ Depth Transparent Prepasses
		pipeline->AddPass(std::make_unique<TransparentDepthTransitionPrepass>());
        pipeline->AddPass(std::make_unique<MeshletTransparentDepthPrepass>(MaterialRenderType::Transparent1Sided));
        pipeline->AddPass(std::make_unique<MeshletTransparentDepthPrepass>(MaterialRenderType::Transparent2Sided));
        pipeline->AddPass(std::make_unique<TraditionalTransparentDepthPrepass>(MaterialRenderType::Transparent1Sided));
        pipeline->AddPass(std::make_unique<TraditionalTransparentDepthPrepass>(MaterialRenderType::Transparent2Sided));

		//Build Hi-Z depth pyramid (Opaque|Transparent)
        pipeline->AddPass(std::make_unique<HizLinearPreparePass>());
        pipeline->AddPass(std::make_unique<HizDownsamplePass>());

		//Light Culling Passes
        pipeline->AddPass(std::make_unique<PointLightCullingPass>());
        pipeline->AddPass(std::make_unique<SpotLightCullingPass>());

        // Deferred Opaque Lighting Passes
        pipeline->AddPass(std::make_unique<DeferredLightTransitionPass>());
        pipeline->AddPass(std::make_unique<DeferredEmissiveAoPass>());
        pipeline->AddPass(std::make_unique<DeferredDirectionLightPass>());
        pipeline->AddPass(std::make_unique<DeferredPointLightPass>());
        pipeline->AddPass(std::make_unique<DeferredSpotLightPass>());

        //Forward+ Cluster Passes
		pipeline->AddPass(std::make_unique<ClusterSetupPass>());
		pipeline->AddPass(std::make_unique<ClusterPointLightCountPass>());
		pipeline->AddPass(std::make_unique<ClusterSpotLightCountPass>());
		pipeline->AddPass(std::make_unique<ClusterPrefixSumPass>());
		pipeline->AddPass(std::make_unique<ClusterPointLightWritePass>());
		pipeline->AddPass(std::make_unique<ClusterSpotLightWritePass>());
		pipeline->AddPass(std::make_unique<ClusterLightWriteSyncPass>());

		//Forward+ Opaque Lighting Passes
        pipeline->AddPass(std::make_unique<OpaqueForwardTransitionPass>());
		pipeline->AddPass(std::make_unique<MeshletOpaqueForwardPass>(MaterialRenderType::Opaque1Sided));
		pipeline->AddPass(std::make_unique<MeshletOpaqueForwardPass>(MaterialRenderType::Opaque2Sided));
		pipeline->AddPass(std::make_unique<TraditionalOpaqueForwardPass>(MaterialRenderType::Opaque1Sided));
		pipeline->AddPass(std::make_unique<TraditionalOpaqueForwardPass>(MaterialRenderType::Opaque2Sided));

		//Forward+ Transparent Lighting Passes (WBOIT)
		pipeline->AddPass(std::make_unique<TransparentForwardTransitionPass>());
        pipeline->AddPass(std::make_unique<MeshletTransparentForwardPass>(MaterialRenderType::Transparent1Sided));
		pipeline->AddPass(std::make_unique<MeshletTransparentForwardPass>(MaterialRenderType::Transparent2Sided));
		pipeline->AddPass(std::make_unique<TraditionalTransparentForwardPass>(MaterialRenderType::Transparent1Sided));
		pipeline->AddPass(std::make_unique<TraditionalTransparentForwardPass>(MaterialRenderType::Transparent2Sided));

		//Transparent Composite Passes (WBOIT)
        pipeline->AddPass(std::make_unique<TransparentCompositeTransitionPass>());
        pipeline->AddPass(std::make_unique<TransparentCompositePass>());

		//Billboard Passes
        /*
        pipeline->AddPass(std::make_unique<BillboardTransitionPass>());
        pipeline->AddPass(std::make_unique<CameraBillboardPass>());
        pipeline->AddPass(std::make_unique<DirectionLightBillboardPass>());
        pipeline->AddPass(std::make_unique<PointLightBillboardPass>());
        pipeline->AddPass(std::make_unique<SpotLightBillboardPass>());

        // Wireframe Passes
        pipeline->AddPass(std::make_unique<WireframeMeshSetupPass>());
        pipeline->AddPass(std::make_unique<WireframeMeshAabbPass>());
        pipeline->AddPass(std::make_unique<WireframeMeshSpherePass>());
        pipeline->AddPass(std::make_unique<PointLightAabbWireframePass>());
        pipeline->AddPass(std::make_unique<PointLightSphereWireframePass>());
        pipeline->AddPass(std::make_unique<SpotLightAabbWireframePass>());
        pipeline->AddPass(std::make_unique<SpotLightSphereWireframePass>());
        */

        // Bloom Post-processing passes
        pipeline->AddPass(std::make_unique<BloomPrefilterPass>());
        pipeline->AddPass(std::make_unique<BloomDownsamplePass>());
        pipeline->AddPass(std::make_unique<BloomUpsamplePass>());
        pipeline->AddPass(std::make_unique<BloomCompositePass>());

		//Debug Visibility Pass
        pipeline->AddPass(std::make_unique<DebugVisibilityPass>());     

		//Gui and Present Passes
		pipeline->AddPass(std::make_unique<PresentationTransitionPass>());
        pipeline->AddPass(std::make_unique<GuiPass>());
        pipeline->InitializeAll();

        renderManager->RegisterPipeline(RenderPipelineNames::DeferredPipeline, std::move(pipeline));

        rtManager->CreateGroup(RenderTargetGroupNames::Deferred);

        uint32_t initWidth = 4;
        uint32_t initHeight = 4;

        Vk::ImageConfig mainImageSpec{};
        mainImageSpec.width = initWidth;
        mainImageSpec.height = initHeight;
        mainImageSpec.type = VK_IMAGE_TYPE_2D;
        mainImageSpec.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        mainImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        mainImageSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::Main, mainImageSpec);

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

        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::ColorMetallic, colorImageSpec);


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

        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::NormalRoughness, normalImageSpec);

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

        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::EmissiveAo, emissiveAoImageSpec);

        Vk::ImageConfig transparentAccumSpec{};
        transparentAccumSpec.width = initWidth;
        transparentAccumSpec.height = initHeight;
        transparentAccumSpec.type = VK_IMAGE_TYPE_2D;
        transparentAccumSpec.format = VK_FORMAT_R16G16B16A16_SFLOAT;
        transparentAccumSpec.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        transparentAccumSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::TransparentAccum, transparentAccumSpec);

        Vk::ImageConfig transparentRevealSpec{};
        transparentRevealSpec.width = initWidth;
        transparentRevealSpec.height = initHeight;
        transparentRevealSpec.type = VK_IMAGE_TYPE_2D;
        transparentRevealSpec.format = VK_FORMAT_R8_UNORM; 
        transparentRevealSpec.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        transparentRevealSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::TransparentReveal, transparentRevealSpec);

        Vk::ImageConfig entityImageSpec{};
        entityImageSpec.width = initWidth;
        entityImageSpec.height = initHeight;
        entityImageSpec.type = VK_IMAGE_TYPE_2D;
        entityImageSpec.format = VK_FORMAT_R32G32_UINT;
        entityImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        entityImageSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::EntityIndex, entityImageSpec);

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

        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::DepthPyramid, depthPyramidImageSpec);

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

        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::Bloom, bloomImageSpec);

        Vk::ImageConfig opaqueDepthSpec{};
        opaqueDepthSpec.width = initWidth;
        opaqueDepthSpec.height = initHeight;
        opaqueDepthSpec.type = VK_IMAGE_TYPE_2D;
        opaqueDepthSpec.format = VK_FORMAT_D32_SFLOAT;
        opaqueDepthSpec.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        opaqueDepthSpec.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::OpaqueDepth, opaqueDepthSpec);

        Vk::ImageConfig transparentDepthSpec{};
        transparentDepthSpec.width = initWidth;
        transparentDepthSpec.height = initHeight;
        transparentDepthSpec.type = VK_IMAGE_TYPE_2D;
        transparentDepthSpec.format = VK_FORMAT_D32_SFLOAT;
        transparentDepthSpec.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        transparentDepthSpec.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::TransparentDepth, transparentDepthSpec);

        return renderManager;
    }
}