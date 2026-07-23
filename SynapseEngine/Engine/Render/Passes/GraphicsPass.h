#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Rendering/RenderUtils.h"
#include "Engine/Vk/Shader/ShaderProgram.h"
#include <string>
#include <vector>
#include <memory>

#include "../IRenderPass.h"

namespace Syn
{
    class SYN_API GraphicsPass : public IRenderPass
    {
    public:
        virtual void Execute(const RenderContext& context) override;
    protected:
        virtual void PrepareFrame(const RenderContext& context) {}
        virtual void BindDescriptors(const RenderContext& context) {}
        virtual void PushConstants(const RenderContext& context) {}
        virtual void Draw(const RenderContext& context) {};
    protected:
        bool _useDynamicRendering = true;
        uint32_t _shaderProgramId = UINT32_MAX;
        Vk::ShaderProgram* _shaderProgram = nullptr;;
        Vk::GraphicsPipelineConfig _graphicsState;
        std::vector<PassImageTransition> _imageTransitions;
        std::vector<VkRenderingAttachmentInfo> _colorAttachments;
        std::optional<VkRenderingAttachmentInfo> _depthAttachment;
        std::optional<VkRenderingAttachmentInfo> _stencilAttachment;
        std::optional<Vk::RenderingInfoConfig> _renderInfo;
    };
}