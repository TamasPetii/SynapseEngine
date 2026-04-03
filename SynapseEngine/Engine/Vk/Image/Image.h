#pragma once
#include "../VkCommon.h"
#include "ImageConfig.h"

namespace Syn::Vk 
{
    class ImageFactory;

    class SYN_API Image {
    public:
        explicit Image(const ImageConfig& config);
        Image(VkImage existingHandle, const ImageConfig& config); //Swapchain wrapper!
        ~Image();

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
        Image(Image&&) = delete;
        Image& operator=(Image&&) = delete;

        VkImage Handle() const { return _handle; }
        VkFormat GetFormat() const { return _config.format; }
        VkExtent3D GetExtent() const { return { _config.width, _config.height, _config.depth }; }
        const ImageConfig& GetConfig() const { return _config; }
        VkImageView GetView(const std::string& name = "") const;



        void TransitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout, VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess, bool discardContent = false);
    
        void OverrideInternalState(VkImageLayout layout, VkPipelineStageFlags2 stage, VkAccessFlags2 access) {
            _currentLayout = layout;
            _currentStage = stage;
            _currentAccess = access;
        }
    private:
        ImageConfig _config;

        VkImage _handle = VK_NULL_HANDLE;
        VmaAllocator _allocator = VK_NULL_HANDLE;
        VmaAllocation _allocation = VK_NULL_HANDLE;

        VkImageLayout _currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkPipelineStageFlags2 _currentStage = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 _currentAccess = VK_ACCESS_2_NONE;

        std::unordered_map<std::string, VkImageView> _imageViews;

        friend class ImageFactory;
    };
}