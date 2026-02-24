#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Rendering/RenderUtils.h"
#include "Engine/Vk/Shader/ShaderProgram.h"
#include "Engine/Vk/Image/Image.h"

#include <string>
#include <vector>
#include <memory>

#include "RenderScene.h"

namespace Syn 
{
    struct ResourceManager;

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
        virtual void Initialize(std::shared_ptr<ResourceManager> resourceManager) = 0;
        virtual void Execute(VkCommandBuffer cmd, const RenderScene& scene) = 0;
        virtual std::string GetName() const = 0;
    };
}

