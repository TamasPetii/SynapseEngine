#include "CameraBillboardPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Render/RenderNames.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Image/SamplerNames.h"
#include "Engine/Vk/Buffer/BufferUtils.h"
#include "Engine/Vk/Descriptor/PushDescriptorWriter.h"
#include "Engine/Component/CameraComponent.h"

namespace Syn {

    #include "Engine/Shaders/Includes/PushConstants/BillboardPC.glsl"

    bool CameraBillboardPass::ShouldExecute(const RenderContext& context) const {
        auto registry = context.scene->GetRegistry();
        if (!registry) return false;

        auto pool = registry->GetPool<CameraComponent>();
        if (!pool || pool->Size() == 0) return false;

        return context.scene->GetSettings()->enableBillboardCameras;
    }

    void CameraBillboardPass::Initialize() {
        auto shaderManager = ServiceLocator::GetShaderManager();

        Vk::ShaderProgramConfig config;
        config.useDescriptorBuffers = false;

        _shaderProgram = shaderManager->CreateProgram(
            "CameraBillboardProgram",
            {
                ShaderNames::BillboardVert,
                ShaderNames::BillboardFrag
            },
            config
        );

        _graphicsState = {
            .raster = {
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .cullMode = VK_CULL_MODE_NONE,
                .polygonMode = VK_POLYGON_MODE_FILL
            },
            .depth = {
                .testEnable = VK_TRUE,
                .writeEnable = VK_TRUE,
                .compareOp = VK_COMPARE_OP_LESS
            },
            .blendStates = {
                {
                    .enable = VK_TRUE,
                    .srcColorFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                    .dstColorFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD
                },
                {
                    .enable = VK_FALSE
                }
            },
            .colorAttachmentCount = 2
        };

        _iconTexture = ServiceLocator::GetImageManager()->LoadImageSync("../Assets/CameraIcon.png");
    }

    void CameraBillboardPass::PrepareFrame(const RenderContext& context) {
        auto group = context.renderTargetManager->GetGroup(RenderTargetGroupNames::Deferred, context.frameIndex);
        VkExtent2D extent = { group->GetWidth(), group->GetHeight() };

        _graphicsState.renderArea = extent;

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = group->GetImage(RenderTargetNames::Main)->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            }));

        _colorAttachments.push_back(Vk::RenderUtils::CreateAttachment({
            .imageView = group->GetImage(RenderTargetNames::EntityIndex)->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            }));

        _depthAttachment = Vk::RenderUtils::CreateAttachment({
            .imageView = group->GetImage(RenderTargetNames::EditorPickingDepth)->GetView(Vk::ImageViewNames::Default),
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
            });

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = &_depthAttachment.value(),
            .layerCount = 1
        };
    }

    void CameraBillboardPass::BindDescriptors(const RenderContext& context) {
        auto imageManager = ServiceLocator::GetImageManager();
        auto texture = imageManager->GetResource(_iconTexture);
        auto sampler = imageManager->GetSampler(SamplerNames::LinearClampEdge)->Handle();

        Vk::PushDescriptorWriter pushWriter;
        pushWriter.AddCombinedImageSampler(
            0,
            texture->image->GetView(Vk::ImageViewNames::Default),
            sampler,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

        pushWriter.Push(context.cmd, _shaderProgram->GetLayout(), 2, VK_PIPELINE_BIND_POINT_GRAPHICS);
    }

    void CameraBillboardPass::PushConstants(const RenderContext& context) {
        auto scene = context.scene;
        auto compManager = scene->GetComponentBufferManager();
        uint32_t fIdx = context.frameIndex;

        BillboardPC pc{};
        pc.cameraBufferAddr = compManager->GetBufferAddr(BufferNames::CameraData, fIdx);
        pc.cameraSparseMapAddr = compManager->GetBufferAddr(BufferNames::CameraSparseMap, fIdx);
        pc.transformBufferAddr = compManager->GetBufferAddr(BufferNames::TransformData, fIdx);
        pc.transformSparseMapAddr = compManager->GetBufferAddr(BufferNames::TransformSparseMap, fIdx);
        pc.visibleEntitiesAddr = compManager->GetBufferAddr(BufferNames::CameraVisibleData, fIdx);
        pc.activeCameraEntity = scene->GetSettings()->useDebugCamera ? scene->GetDebugCameraEntity() : scene->GetSceneCameraEntity();
        pc.baseScale = 1.0f;

        vkCmdPushConstants(context.cmd, _shaderProgram->GetLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(BillboardPC), &pc);
    }

    void CameraBillboardPass::Draw(const RenderContext& context) {
        auto registry = context.scene->GetRegistry();
        auto pool = registry->GetPool<CameraComponent>();

        if (!pool || pool->Size() == 0) return;

        uint32_t cameraCount = static_cast<uint32_t>(pool->Size());
        vkCmdDraw(context.cmd, 6, cameraCount, 0, 0);
    }
}