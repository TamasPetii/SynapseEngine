#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Rendering/RenderUtils.h"
#include "Engine/Vk/Shader/ShaderProgram.h"
#include "Engine/Vk/Image/Image.h"

#include <string>
#include <vector>
#include <memory>

#include "RenderContext.h"

namespace Syn 
{
    struct SYN_API PassImageTransition
    {
        Vk::Image* image;
        VkImageLayout newLayout;
        VkPipelineStageFlags2 dstStage;
        VkAccessFlags2 dstAccess;
        bool discardContent = false;
    };

    class SYN_API IRenderPass
    {
    public:
        virtual ~IRenderPass() = default;
        virtual void Initialize() = 0;
        virtual void Execute(const RenderContext& context) = 0;
        virtual std::string GetName() const = 0;
    };
}

