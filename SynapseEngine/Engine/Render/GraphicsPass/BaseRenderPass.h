#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Rendering/RenderUtils.h"
#include "Engine/Vk/Shader/ShaderProgram.h"
#include <string>
#include <vector>
#include <memory>

#include "../Data/RenderScene.h"
#include "../IRenderPass.h"

namespace Syn
{
    class SYN_API BaseRenderPass : public IRenderPass
    {
    public:
        void Execute(const RenderContext& context);
    protected:
        virtual void PrepareFrame(const RenderContext& context) {}
        virtual void BindDescriptors(const RenderContext& context) {}
        virtual void PushConstants(const RenderContext& context) {}
        virtual void Draw(const RenderContext& context) {};
    protected:
        bool _useDynamicRendering = true;
        Vk::ShaderProgram* _shaderProgram;
        Vk::GraphicsPipelineConfig _graphicsState;
        std::vector<PassImageTransition> _imageTransitions;
        std::vector<VkRenderingAttachmentInfo> _colorAttachments;
        std::optional<VkRenderingAttachmentInfo> _depthAttachment;
        std::optional<VkRenderingAttachmentInfo> _stencilAttachment;
        std::optional<Vk::RenderingInfoConfig> _renderInfo;
    };
}