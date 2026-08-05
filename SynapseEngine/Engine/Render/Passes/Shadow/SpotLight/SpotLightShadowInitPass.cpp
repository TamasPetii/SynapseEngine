#include "SpotLightShadowInitPass.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Vk/Image/ImageFactory.h"
#include "Engine/Scene/BufferNames.h"
#include "Engine/Manager/ComponentBufferManager.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Vk/Image/ImageViewNames.h"
#include "Engine/Scene/DrawData/SpotLightShadowDrawGroup.h"

namespace Syn {

    void SpotLightShadowInitPass::PrepareFrame(const RenderContext& context)
    {
        auto drawData = context.scene->GetSceneDrawData();
        auto fIdx = context.frameIndex;

        VkExtent2D extent = { SPOT_SHADOW_ATLAS_SIZE, SPOT_SHADOW_ATLAS_SIZE };
        _graphicsState.renderArea = extent;

        if (auto depthImg = drawData->SpotLightShadow.shadowAtlas[fIdx].get())
        {
            _depthAttachment = Vk::RenderUtils::CreateAttachment({
                .imageView = depthImg->GetView(Vk::ImageViewNames::Default),
                .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .clearValue = VkClearValue{.depthStencil = {1.0f, 0}},
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                });

            _imageTransitions.push_back({
                .image = depthImg,
                .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .dstStage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                .dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .discardContent = true
                });
        }

        _renderInfo = Vk::RenderingInfoConfig{
            .renderArea = extent,
            .colorAttachments = _colorAttachments,
            .depthAttachment = _depthAttachment.has_value() ? &_depthAttachment.value() : nullptr,
            .layerCount = 1
        };
    }
}