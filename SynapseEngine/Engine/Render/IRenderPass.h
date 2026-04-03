#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Rendering/RenderUtils.h"
#include "Engine/Vk/Shader/ShaderProgram.h"
#include "Engine/Vk/Image/Image.h"

#include <string>
#include <vector>
#include <memory>

#include "RenderContext.h"
#include "ShaderNames.h"
#include "RenderNames.h"

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
        virtual void Initialize() {};
        virtual void Execute(const RenderContext& context) {};
        virtual std::string GetName() const = 0;      
        virtual bool ShouldExecute(const RenderContext& context) const { return true; }
    };
}

