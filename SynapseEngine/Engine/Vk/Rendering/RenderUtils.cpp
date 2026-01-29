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

        VkBool32 blendEnable = config.blend.enable ? VK_TRUE : VK_FALSE;
        std::vector<VkBool32> blendEnables(config.colorAttachmentCount, blendEnable);
        vkCmdSetColorBlendEnableEXT(cmd, 0, config.colorAttachmentCount, blendEnables.data());

        if (config.blend.enable) {
            VkColorBlendEquationEXT equation{};
            equation.srcColorBlendFactor = config.blend.srcColorFactor;
            equation.dstColorBlendFactor = config.blend.dstColorFactor;
            equation.colorBlendOp = config.blend.colorBlendOp;
            equation.srcAlphaBlendFactor = config.blend.srcAlphaFactor;
            equation.dstAlphaBlendFactor = config.blend.dstAlphaFactor;
            equation.alphaBlendOp = config.blend.alphaBlendOp;

            std::vector<VkColorBlendEquationEXT> equations(config.colorAttachmentCount, equation);
            vkCmdSetColorBlendEquationEXT(cmd, 0, config.colorAttachmentCount, equations.data());
        }

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
    }

    void RenderUtils::BindShaders(VkCommandBuffer cmd, std::span<const Shader* const> shaders) {
        std::vector<VkShaderStageFlagBits> stages;
        std::vector<VkShaderEXT> shaderHandles;

        stages.reserve(shaders.size());
        shaderHandles.reserve(shaders.size());

        for (const auto* shader : shaders) {
            if (shader) {
                stages.push_back(shader->GetStage());
                shaderHandles.push_back(shader->Handle());
            }
        }

        if (!stages.empty()) {
            vkCmdBindShadersEXT(
                cmd,
                static_cast<uint32_t>(stages.size()),
                stages.data(),
                shaderHandles.data()
            );
        }
    }
}