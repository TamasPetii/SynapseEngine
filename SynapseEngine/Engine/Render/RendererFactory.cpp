#include "RendererFactory.h"
#include "Engine/Vk/Image/ImageConfig.h"

#include "Engine/Render/Passes/Culling/ModelCullingPass.h"
#include "Engine/Render/Passes/Culling/MeshCullingPass.h"
#include "Engine/Render/Passes/Culling/PointLightCullingPass.h"
#include "Engine/Render/Passes/Culling/SpotLightCullingPass.h"

#include "Engine/Render/Passes/Deferred/GBufferInitPass.h"
#include "Engine/Render/Passes/Deferred/DeferredEmissiveAoPass.h"
#include "Engine/Render/Passes/Deferred/DeferredPointLightPass.h"
#include "Engine/Render/Passes/Deferred/DeferredSpotLightPass.h"
#include "Engine/Render/Passes/Deferred/DeferredDirectionLightPass.h"

#include "Engine/Render/Passes/Geometry/TraditionalOpaquePass.h"
#include "Engine/Render/Passes/Geometry/MeshletOpaquePass.h"
#include "Engine/Render/Passes/Geometry/TraditionalTransparentPass.h"
#include "Engine/Render/Passes/Geometry/MeshletTransparentPass.h"

#include "Engine/Render/Passes/Wboit/WboitInitPass.h"
#include "Engine/Render/Passes/Wboit/TransparentCompositePass.h"

#include "Engine/Render/Passes/Hiz/HizLinearPreparePass.h"
#include "Engine/Render/Passes/Hiz/HizDownsamplePass.h"

#include "Engine/Render/Passes/Bloom/BloomPrefilterPass.h"
#include "Engine/Render/Passes/Bloom/BloomUpsamplePass.h"
#include "Engine/Render/Passes/Bloom/BloomDownsamplePass.h"
#include "Engine/Render/Passes/Bloom/BloomCompositePass.h"

#include "Engine/Render/Passes/Picking/DepthCopyPass.h"
#include "Engine/Render/Passes/Picking/MeshletTransparentPickingPass.h"
#include "Engine/Render/Passes/Picking/TraditionalTransparentPickingPass.h"

#include "Engine/Render/Passes/Wireframe/DebugInitPass.h"
#include "Engine/Render/Passes/Wireframe/WireframeSetupPass.h"
#include "Engine/Render/Passes/Wireframe/WireframeAabbPass.h"
#include "Engine/Render/Passes/Wireframe/WireframeSpherePass.h"
#include "Engine/Render/Passes/Wireframe/WireframeMeshletInitPass.h"
#include "Engine/Render/Passes/Wireframe/WireframeMeshletAabbPass.h"
#include "Engine/Render/Passes/Wireframe/WireframeMeshletSpherePass.h"
#include "Engine/Render/Passes/Wireframe/PointLightAabbWireframePass.h"
#include "Engine/Render/Passes/Wireframe/PointLightSphereWireframePass.h"
#include "Engine/Render/Passes/Wireframe/SpotLightAabbWireframePass.h"
#include "Engine/Render/Passes/Wireframe/SpotLightSphereWireframePass.h"

#include "Engine/Render/Passes/Present/GuiPass.h"
#include "Engine/Render/Passes/Present/CompositePass.h"
#include "Engine/Render/Passes/Present/PresentationPass.h"
#include "Engine/Render/Passes/Present/CopyToSwapchainPass.h"

#include "Engine/Vk/Image/ImageViewNames.h"
#include "RenderNames.h"

namespace Syn 
{
    std::unique_ptr<RenderManager> RendererFactory::CreateDeferredRenderer(uint32_t framesInFlight) {
        auto renderManager = std::make_unique<RenderManager>(framesInFlight);
        auto rtManager = renderManager->GetRenderTargetManager();

        auto pipeline = std::make_unique<RenderPipeline>();
        pipeline->AddPass(std::make_unique<HizLinearPreparePass>());
        pipeline->AddPass(std::make_unique<HizDownsamplePass>());
        pipeline->AddPass(std::make_unique<ModelCullingPass>());
        pipeline->AddPass(std::make_unique<MeshCullingPass>());
        pipeline->AddPass(std::make_unique<PointLightCullingPass>());
        pipeline->AddPass(std::make_unique<SpotLightCullingPass>());

        //Texture Init Passes
        pipeline->AddPass(std::make_unique<GBufferInitPass>());
        pipeline->AddPass(std::make_unique<WboitInitPass>());
        pipeline->AddPass(std::make_unique<DebugInitPass>());

        // Opaque Passes
        pipeline->AddPass(std::make_unique<TraditionalOpaquePass>(MaterialRenderType::Opaque1Sided));
        pipeline->AddPass(std::make_unique<TraditionalOpaquePass>(MaterialRenderType::Opaque2Sided));
        pipeline->AddPass(std::make_unique<MeshletOpaquePass>(MaterialRenderType::Opaque1Sided));
        pipeline->AddPass(std::make_unique<MeshletOpaquePass>(MaterialRenderType::Opaque2Sided));

		// Deferred Lighting Passes
		pipeline->AddPass(std::make_unique<DeferredEmissiveAoPass>());
        pipeline->AddPass(std::make_unique<DeferredDirectionLightPass>());
        pipeline->AddPass(std::make_unique<DeferredPointLightPass>());
        pipeline->AddPass(std::make_unique<DeferredSpotLightPass>());

        // Editor Picking Passes
        pipeline->AddPass(std::make_unique<DepthCopyPass>());
        pipeline->AddPass(std::make_unique<TraditionalTransparentPickingPass>(MaterialRenderType::Transparent1Sided));
        pipeline->AddPass(std::make_unique<TraditionalTransparentPickingPass>(MaterialRenderType::Transparent2Sided));
        pipeline->AddPass(std::make_unique<MeshletTransparentPickingPass>(MaterialRenderType::Transparent1Sided));
        pipeline->AddPass(std::make_unique<MeshletTransparentPickingPass>(MaterialRenderType::Transparent2Sided));

        // WBOIT Transparent Passes
        pipeline->AddPass(std::make_unique<TraditionalTransparentPass>(MaterialRenderType::Transparent1Sided));
        pipeline->AddPass(std::make_unique<TraditionalTransparentPass>(MaterialRenderType::Transparent2Sided));
        pipeline->AddPass(std::make_unique<MeshletTransparentPass>(MaterialRenderType::Transparent1Sided));
        pipeline->AddPass(std::make_unique<MeshletTransparentPass>(MaterialRenderType::Transparent2Sided));
        pipeline->AddPass(std::make_unique<TransparentCompositePass>());

        // Wireframe Passes
        pipeline->AddPass(std::make_unique<WireframeSetupPass>());
        pipeline->AddPass(std::make_unique<WireframeAabbPass>());
        pipeline->AddPass(std::make_unique<WireframeSpherePass>());
        //pipeline->AddPass(std::make_unique<WireframeMeshletInitPass>());
        //pipeline->AddPass(std::make_unique<WireframeMeshletAabbPass>());
        //pipeline->AddPass(std::make_unique<WireframeMeshletSpherePass>());

        pipeline->AddPass(std::make_unique<PointLightAabbWireframePass>());
        pipeline->AddPass(std::make_unique<PointLightSphereWireframePass>());
        pipeline->AddPass(std::make_unique<SpotLightAabbWireframePass>());
        pipeline->AddPass(std::make_unique<SpotLightSphereWireframePass>());

        // Post-processing & UI
        pipeline->AddPass(std::make_unique<BloomPrefilterPass>());
        pipeline->AddPass(std::make_unique<BloomDownsamplePass>());
        pipeline->AddPass(std::make_unique<BloomUpsamplePass>());
        pipeline->AddPass(std::make_unique<BloomCompositePass>());
        pipeline->AddPass(std::make_unique<GuiPass>());
        pipeline->AddPass(std::make_unique<PresentationPass>());
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

        // --- 2. Revealage Textúra (Felfedő) ---
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
        depthPyramidImageSpec.format = VK_FORMAT_R32_SFLOAT;
        depthPyramidImageSpec.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
        depthPyramidImageSpec.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        depthPyramidImageSpec.generateMipMaps = true;

        depthPyramidImageSpec.AddView(Vk::ImageViewNames::Default, Vk::ImageViewConfig{
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
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
        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::Depth, depthImageSpec);

        Vk::ImageConfig pickingDepthSpec{};
        pickingDepthSpec.width = initWidth;
        pickingDepthSpec.height = initHeight;
        pickingDepthSpec.type = VK_IMAGE_TYPE_2D;
        pickingDepthSpec.format = VK_FORMAT_D32_SFLOAT;
        pickingDepthSpec.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        pickingDepthSpec.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        rtManager->AddAttachment(RenderTargetGroupNames::Deferred, RenderTargetNames::EditorPickingDepth, pickingDepthSpec);

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