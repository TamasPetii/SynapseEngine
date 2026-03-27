#include "RendererFactory.h"
#include "Engine/Vk/Image/ImageConfig.h"
#include "Engine/Render/GraphicsPass/GBufferInitPass.h"
#include "Engine/Render/GraphicsPass/TraditionalOpaquePass.h"
#include "Engine/Render/GraphicsPass/MeshletOpaquePass.h"
#include "Engine/Render/TransferPass/PresentationPass.h"
#include "Engine/Render/TransferPass/CopyToSwapchainPass.h"
#include "Engine/Render/ComputePass/ModelCullingPass.h"
#include "Engine/Render/ComputePass/MeshCullingPass.h"
#include "Engine/Render/ComputePass/HizLinearPreparePass.h"
#include "Engine/Render/ComputePass/HizDownsamplePass.h"
#include "Engine/Render/GraphicsPass/CompositePass.h"
#include "Engine/Render/ComputePass/BloomPrefilterPass.h"
#include "Engine/Render/ComputePass/BloomUpsamplePass.h"
#include "Engine/Render/ComputePass/BloomDownsamplePass.h"
#include "Engine/Render/ComputePass/BloomCompositePass.h"
#include "Engine/Render/ComputePass/WireframeSetupPass.h"
#include "Engine/Render/GraphicsPass/WireframeAabbPass.h"
#include "Engine/Render/GraphicsPass/WireframeSpherePass.h"
#include "Engine/Render/TransferPass/WireframeMeshletInitPass.h"
#include "Engine/Render/GraphicsPass/WireframeMeshletAabbPass.h"
#include "Engine/Render/GraphicsPass/WireframeMeshletSpherePass.h"
#include "Engine/Render/GraphicsPass/GuiPass.h"

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
        //pipeline->AddPass(std::make_unique<ModelCullingPass>());
       // pipeline->AddPass(std::make_unique<MeshCullingPass>());
        pipeline->AddPass(std::make_unique<GBufferInitPass>());
        pipeline->AddPass(std::make_unique<TraditionalOpaquePass>(MaterialRenderType::Opaque1Sided));
        pipeline->AddPass(std::make_unique<TraditionalOpaquePass>(MaterialRenderType::Opaque2Sided));
        pipeline->AddPass(std::make_unique<MeshletOpaquePass>(MaterialRenderType::Opaque1Sided));
        pipeline->AddPass(std::make_unique<MeshletOpaquePass>(MaterialRenderType::Opaque2Sided));
        //pipeline->AddPass(std::make_unique<WireframeSetupPass>());
        //pipeline->AddPass(std::make_unique<WireframeAabbPass>());
        //pipeline->AddPass(std::make_unique<WireframeSpherePass>());
        //pipeline->AddPass(std::make_unique<WireframeMeshletInitPass>());
        //pipeline->AddPass(std::make_unique<WireframeMeshletAabbPass>());
        //pipeline->AddPass(std::make_unique<WireframeMeshletSpherePass>());
        //pipeline->AddPass(std::make_unique<BloomPrefilterPass>());
        //pipeline->AddPass(std::make_unique<BloomDownsamplePass>());
        //pipeline->AddPass(std::make_unique<BloomUpsamplePass>());
        //pipeline->AddPass(std::make_unique<BloomCompositePass>());
        //pipeline->AddPass(std::make_unique<CompositePass>());
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

        return renderManager;
    }
}