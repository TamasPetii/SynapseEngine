#include "RenderUtils.h"
#include "Engine/Vk/Shader/Shader.h"

namespace Syn::Vk {

    VkRenderingAttachmentInfo RenderUtils::CreateAttachment(const AttachmentConfig& config) {
        VkRenderingAttachmentInfo info{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        info.imageView = config.imageView;
        info.imageLayout = config.layout;
        info.storeOp = config.storeOp;

        if (config.clearValue.has_value()) {
            info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            info.clearValue = config.clearValue.value();
        }
        else {
            info.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        }

        return info;
    }

    void RenderUtils::BeginRendering(VkCommandBuffer cmd, const RenderingInfoConfig& config) {
        VkRenderingInfo renderInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
        renderInfo.renderArea.offset = { 0, 0 };
        renderInfo.renderArea.extent = config.renderArea;
        renderInfo.layerCount = config.layerCount;
        renderInfo.colorAttachmentCount = static_cast<uint32_t>(config.colorAttachments.size());
        renderInfo.pColorAttachments = config.colorAttachments.data();
        renderInfo.pDepthAttachment = config.depthAttachment;
        renderInfo.pStencilAttachment = config.stencilAttachment;

        vkCmdBeginRendering(cmd, &renderInfo);
    }

    void RenderUtils::EndRendering(VkCommandBuffer cmd) {
        vkCmdEndRendering(cmd);
    }

    void RenderUtils::SetGraphicsState(VkCommandBuffer cmd, const GraphicsPipelineConfig& config) {
        vkCmdSetPrimitiveTopology(cmd, config.raster.topology);
        vkCmdSetCullMode(cmd, config.raster.cullMode);
        vkCmdSetFrontFace(cmd, config.raster.frontFace);
        vkCmdSetPolygonModeEXT(cmd, config.raster.polygonMode);
        vkCmdSetLineWidth(cmd, config.raster.lineWidth);

        vkCmdSetRasterizerDiscardEnable(cmd, VK_FALSE);
        vkCmdSetDepthBiasEnable(cmd, VK_FALSE);

        vkCmdSetDepthTestEnable(cmd, config.depth.testEnable);
        vkCmdSetDepthWriteEnable(cmd, config.depth.writeEnable);
        vkCmdSetDepthCompareOp(cmd, config.depth.compareOp);

        vkCmdSetDepthBoundsTestEnable(cmd, VK_FALSE);
        vkCmdSetStencilTestEnable(cmd, VK_FALSE);

        VkColorComponentFlags writeMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        std::vector<VkColorComponentFlags> writeMasks(config.colorAttachmentCount, writeMask);
        vkCmdSetColorWriteMaskEXT(cmd, 0, config.colorAttachmentCount, writeMasks.data());

        std::vector<VkBool32> blendEnables(config.colorAttachmentCount, VK_FALSE);
        for (uint32_t i = 0; i < config.colorAttachmentCount; ++i) {
            if (i < config.blendStates.size()) {
                blendEnables[i] = config.blendStates[i].enable ? VK_TRUE : VK_FALSE;
            }
        }
        vkCmdSetColorBlendEnableEXT(cmd, 0, config.colorAttachmentCount, blendEnables.data());

        vkCmdSetRasterizationSamplesEXT(cmd, config.raster.samples);

        VkSampleMask mask = 0xFFFFFFFF;
        vkCmdSetSampleMaskEXT(cmd, config.raster.samples, &mask);

        vkCmdSetAlphaToCoverageEnableEXT(cmd, config.raster.alphaToCoverageEnable);
        vkCmdSetAlphaToOneEnableEXT(cmd, VK_FALSE);

        vkCmdSetPrimitiveRestartEnable(cmd, config.raster.primitiveRestartEnable);

        std::vector<VkColorBlendEquationEXT> equations(config.colorAttachmentCount);
        for (uint32_t i = 0; i < config.colorAttachmentCount; ++i) {
            if (i < config.blendStates.size() && config.blendStates[i].enable) {
                equations[i].srcColorBlendFactor = config.blendStates[i].srcColorFactor;
                equations[i].dstColorBlendFactor = config.blendStates[i].dstColorFactor;
                equations[i].colorBlendOp = config.blendStates[i].colorBlendOp;
                equations[i].srcAlphaBlendFactor = config.blendStates[i].srcAlphaFactor;
                equations[i].dstAlphaBlendFactor = config.blendStates[i].dstAlphaFactor;
                equations[i].alphaBlendOp = config.blendStates[i].alphaBlendOp;
            }
            else {
                equations[i] = { VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD, VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD };
            }
        }
        vkCmdSetColorBlendEquationEXT(cmd, 0, config.colorAttachmentCount, equations.data());

        if (config.renderArea.has_value()) {
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)config.renderArea->width;
            viewport.height = (float)config.renderArea->height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewportWithCount(cmd, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = config.renderArea.value();
            vkCmdSetScissorWithCount(cmd, 1, &scissor);
        }

        vkCmdSetVertexInputEXT(cmd,
            0, nullptr,
            0, nullptr
        );
    }
}