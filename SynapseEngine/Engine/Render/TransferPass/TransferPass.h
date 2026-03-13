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
    class SYN_API TransferPass : public IRenderPass
    {
    public:
        void Execute(const RenderContext& context) override;
    protected:
        virtual void PrepareFrame(const RenderContext& context) {}
        virtual void Transfer(const RenderContext& context) {}
    protected:
        std::vector<PassImageTransition> _imageTransitions;
    };
}