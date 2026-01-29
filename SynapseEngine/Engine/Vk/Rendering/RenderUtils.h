#pragma once
#include "../VkCommon.h"

namespace Syn::Vk {
    class Shader;

    struct RasterState {
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        float lineWidth = 1.0f;
    };

    struct DepthState {
        bool testEnable = true;
        bool writeEnable = true;
        VkCompareOp compareOp = VK_COMPARE_OP_LESS;
    };

    struct BlendState {
        bool enable = false;
        VkBlendFactor srcColorFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        VkBlendFactor dstColorFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        VkBlendOp colorBlendOp = VK_BLEND_OP_ADD;
        VkBlendFactor srcAlphaFactor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor dstAlphaFactor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp alphaBlendOp = VK_BLEND_OP_ADD;
    };

    struct GraphicsPipelineConfig {
        RasterState raster;
        DepthState depth;
        BlendState blend;
        uint32_t colorAttachmentCount = 1;
        std::optional<VkExtent2D> renderArea = std::nullopt;
    };

    struct AttachmentConfig {
        VkImageView imageView;
        VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        std::optional<VkClearValue> clearValue = std::nullopt;
        VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    };

    struct RenderingInfoConfig {
        VkExtent2D renderArea;
        std::span<const VkRenderingAttachmentInfo> colorAttachments;
        const VkRenderingAttachmentInfo* depthAttachment = nullptr;
        const VkRenderingAttachmentInfo* stencilAttachment = nullptr;
        uint32_t layerCount = 1;
    };

    class SYN_API RenderUtils {
    public:
        static VkRenderingAttachmentInfo CreateAttachment(const AttachmentConfig& config);
        static void BeginRendering(VkCommandBuffer cmd, const RenderingInfoConfig& config);
        static void EndRendering(VkCommandBuffer cmd);
        static void SetGraphicsState(VkCommandBuffer cmd, const GraphicsPipelineConfig& config);
        static void BindShaders(VkCommandBuffer cmd, std::span<const Shader* const> shaders);
    };
}