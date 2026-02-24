#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Rendering/RenderUtils.h"
#include "Engine/Vk/Shader/ShaderProgram.h"
#include <string>
#include <vector>
#include <memory>

#include "../RenderScene.h"
#include "../IRenderPass.h"

namespace Syn
{
    class SYN_API BaseComputePass : public IRenderPass
    {
    public:
        void Execute(VkCommandBuffer cmd, const RenderScene& scene);
    protected:
        virtual void BindDescriptors(VkCommandBuffer cmd, const RenderScene& scene) {}
        virtual void PushConstants(VkCommandBuffer cmd, const RenderScene& scene) {}
        virtual void Dispatch(VkCommandBuffer cmd, const RenderScene& scene) = 0;
    protected:
        std::vector<PassImageTransition> _imageTransitions;
        std::shared_ptr<Vk::ShaderProgram> _shaderProgram;
    };
}