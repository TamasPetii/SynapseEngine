#include "RendererFactory.h"
#include "Engine/Vk/Image/ImageConfig.h"

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

#include "Engine/Render/Passes/Hiz/HizLinearPreparePass.h"
#include "Engine/Render/Passes/Hiz/HizDownsamplePass.h"

#include "Engine/Render/Passes/Present/GuiPass.h"
#include "Engine/Render/Passes/Present/CompositePass.h"

#include "Engine/Render/Passes/Setup/GLobalFrameSetupPass.h"

#include "Engine/Render/Passes/Shading/Common/DepthCopyPass.h"
#include "Engine/Render/Passes/Shading/Common/OpaqueInitPass.h"
#include "Engine/Render/Passes/Shading/Common/TransparentInitPass.h"

#include "Engine/Render/Passes/Shading/Deferred/GBuffer/MeshletOpaqueDeferredPass.h"
#include "Engine/Render/Passes/Shading/Deferred/GBuffer/TraditionalOpaqueDeferredPass.h"

#include "Engine/Render/Passes/Shading/Deferred/Lighting/PreDeferredTransitionPass.h"
#include "Engine/Render/Passes/Shading/Deferred/Lighting/DeferredEmissiveAoPass.h"
#include "Engine/Render/Passes/Shading/Deferred/Lighting/DeferredPointLightPass.h"
#include "Engine/Render/Passes/Shading/Deferred/Lighting/DeferredSpotLightPass.h"
#include "Engine/Render/Passes/Shading/Deferred/Lighting/DeferredDirectionLightPass.h"
#include "Engine/Render/Passes/Shading/Deferred/Lighting/PostDeferredTransitionPass.h"

#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterSetupPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterPointLightCountPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterSpotLightCountPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterPrefixSumPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterPointLightWritePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterSpotLightWritePass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Clustering/ClusterLightWriteSyncPass.h"

#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/MeshletOpaqueDepthPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/MeshletTransparentDepthPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/TraditionalOpaqueDepthPrepass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/DepthPrepass/TraditionalTransparentDepthPrepass.h"

#include "Engine/Render/Passes/Shading/ForwardPlus/Lighting/MeshletOpaqueForwardPass.h"
#include "Engine/Render/Passes/Shading/ForwardPlus/Lighting/TraditionalOpaqueForwardPass.h"

#include "Engine/Render/Passes/Shading/Wboit/MeshletTransparentForwardPass.h"
#include "Engine/Render/Passes/Shading/Wboit/TraditionalTransparentForwardPass.h"
#include "Engine/Render/Passes/Shading/Wboit/PreCompositeTransitionPass.h"
#include "Engine/Render/Passes/Shading/Wboit/TransparentCompositePass.h"

#include "Engine/Render/Passes/Wireframe/WireframeMeshSetupPass.h"
#include "Engine/Render/Passes/Wireframe/WireframeMeshAabbPass.h"
#include "Engine/Render/Passes/Wireframe/WireframeMeshSpherePass.h"
#include "Engine/Render/Passes/Wireframe/PointLightAabbWireframePass.h"
#include "Engine/Render/Passes/Wireframe/PointLightSphereWireframePass.h"
#include "Engine/Render/Passes/Wireframe/SpotLightAabbWireframePass.h"
#include "Engine/Render/Passes/Wireframe/SpotLightSphereWireframePass.h"

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

		//Geometry Culling Passes
		pipeline->AddPass(std::make_unique<CullingCommandResetPass>());
        pipeline->AddPass(std::make_unique<ModelCullingPass>());
        pipeline->AddPass(std::make_unique<MeshCullingPass>());

		//Forward+ Depth Opaque Prepasses
		pipeline->AddPass(std::make_unique<MeshletOpaqueDepthPrepass>(MaterialRenderType::Opaque1Sided));
		pipeline->AddPass(std::make_unique<MeshletOpaqueDepthPrepass>(MaterialRenderType::Opaque2Sided));
		pipeline->AddPass(std::make_unique<TraditionalOpaqueDepthPrepass>(MaterialRenderType::Opaque1Sided));
		pipeline->AddPass(std::make_unique<TraditionalOpaqueDepthPrepass>(MaterialRenderType::Opaque2Sided));

        //Deferred Depth Opaque Prepasses
		pipeline->AddPass(std::make_unique<MeshletOpaqueDeferredPass>(MaterialRenderType::Opaque1Sided));
        pipeline->AddPass(std::make_unique<MeshletOpaqueDeferredPass>(MaterialRenderType::Opaque2Sided));
		pipeline->AddPass(std::make_unique<TraditionalOpaqueDeferredPass>(MaterialRenderType::Opaque1Sided));
		pipeline->AddPass(std::make_unique<TraditionalOpaqueDeferredPass>(MaterialRenderType::Opaque2Sided));

        //Copy Opaque Depth to Transparent Depth Buffer
		pipeline->AddPass(std::make_unique<DepthCopyPass>());

        //Forward+ Depth Transparent Prepasses
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

        //Forward+ Cluster Passes
		pipeline->AddPass(std::make_unique<ClusterSetupPass>());
		pipeline->AddPass(std::make_unique<ClusterPointLightCountPass>());
		pipeline->AddPass(std::make_unique<ClusterSpotLightCountPass>());
		pipeline->AddPass(std::make_unique<ClusterPrefixSumPass>());
		pipeline->AddPass(std::make_unique<ClusterPointLightWritePass>());
		pipeline->AddPass(std::make_unique<ClusterSpotLightWritePass>());
		pipeline->AddPass(std::make_unique<ClusterLightWriteSyncPass>());

		// Deferred Opaque Lighting Passes
		pipeline->AddPass(std::make_unique<PreDeferredTransitionPass>());
		pipeline->AddPass(std::make_unique<DeferredEmissiveAoPass>());
        pipeline->AddPass(std::make_unique<DeferredDirectionLightPass>());
        pipeline->AddPass(std::make_unique<DeferredPointLightPass>());
        pipeline->AddPass(std::make_unique<DeferredSpotLightPass>());
		pipeline->AddPass(std::make_unique<PostDeferredTransitionPass>());

		//Forward+ Opaque Lighting Passes
		pipeline->AddPass(std::make_unique<MeshletOpaqueForwardPass>(MaterialRenderType::Opaque1Sided));
		pipeline->AddPass(std::make_unique<MeshletOpaqueForwardPass>(MaterialRenderType::Opaque2Sided));
		pipeline->AddPass(std::make_unique<TraditionalOpaqueForwardPass>(MaterialRenderType::Opaque1Sided));
		pipeline->AddPass(std::make_unique<TraditionalOpaqueForwardPass>(MaterialRenderType::Opaque2Sided));

		//Forward+ Transparent Lighting Passes (WBOIT)
		pipeline->AddPass(std::make_unique<MeshletTransparentForwardPass>(MaterialRenderType::Transparent1Sided));
		pipeline->AddPass(std::make_unique<MeshletTransparentForwardPass>(MaterialRenderType::Transparent2Sided));
		pipeline->AddPass(std::make_unique<TraditionalTransparentForwardPass>(MaterialRenderType::Transparent1Sided));
		pipeline->AddPass(std::make_unique<TraditionalTransparentForwardPass>(MaterialRenderType::Transparent2Sided));
        pipeline->AddPass(std::make_unique<PreCompositeTransitionPass>());
        pipeline->AddPass(std::make_unique<TransparentCompositePass>());

		//Billboard Passes
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

        // Bloom Post-processing passes
        pipeline->AddPass(std::make_unique<BloomPrefilterPass>());
        pipeline->AddPass(std::make_unique<BloomDownsamplePass>());
        pipeline->AddPass(std::make_unique<BloomUpsamplePass>());
        pipeline->AddPass(std::make_unique<BloomCompositePass>());

		//Gui and Present Passes
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
        entityImageSpec.format = VK_FORMAT_R32_UINT;
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


        Vk::ImageConfig depthImageSpec{};
        depthImageSpec.width = initWidth;
        depthImageSpec.height = initHeight;
        depthImageSpec.type = VK_IMAGE_TYPE_2D;
        depthImageSpec.format = VK_FORMAT_D32_SFLOAT;
        depthImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        depthImageSpec.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::OpaqueDepth, depthImageSpec);

        Vk::ImageConfig pickingDepthSpec{};
        pickingDepthSpec.width = initWidth;
        pickingDepthSpec.height = initHeight;
        pickingDepthSpec.type = VK_IMAGE_TYPE_2D;
        pickingDepthSpec.format = VK_FORMAT_D32_SFLOAT;
        pickingDepthSpec.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        pickingDepthSpec.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::TransparentDepth, pickingDepthSpec);

        Vk::ImageConfig debugImageSpec{};
        debugImageSpec.width = initWidth;
        debugImageSpec.height = initHeight;
        debugImageSpec.type = VK_IMAGE_TYPE_2D;
        debugImageSpec.format = VK_FORMAT_R8G8B8A8_UNORM;
        debugImageSpec.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        debugImageSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        Vk::ImageConfig debugGeometryPipelineSpec = debugImageSpec;
        debugGeometryPipelineSpec.AddView(RenderTargetViewNames::DebugTopology, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .mipLevelCount = 1,
            .swizzle = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_ONE }
            });
        debugGeometryPipelineSpec.AddView(RenderTargetViewNames::DebugPipeline, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .mipLevelCount = 1,
            .swizzle = { VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_ONE }
            });
        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::DebugTopologyPipeline, debugGeometryPipelineSpec);

        Vk::ImageConfig debugLodSpec = debugImageSpec;
        debugLodSpec.AddView(RenderTargetViewNames::DebugMeshlet, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .mipLevelCount = 1,
            .swizzle = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_ONE }
            });
        debugLodSpec.AddView(RenderTargetViewNames::DebugLodGrayscale, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .mipLevelCount = 1,
            .swizzle = { VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_ONE }
            });
        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::DebugMeshletLod, debugLodSpec);

        Vk::ImageConfig debugMaterialUvSpec = debugImageSpec;
        debugMaterialUvSpec.AddView(RenderTargetViewNames::DebugMaterial, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .mipLevelCount = 1,
            .swizzle = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ONE }
            });
        debugMaterialUvSpec.AddView(RenderTargetViewNames::DebugUv, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .mipLevelCount = 1,
            .swizzle = { VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ONE }
            });
        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::DebugMaterialUv, debugMaterialUvSpec);

        return renderManager;
    }
}