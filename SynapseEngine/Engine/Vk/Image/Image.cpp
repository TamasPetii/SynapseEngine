#include "Image.h"
#include "ImageFactory.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"
#include "ImageUtils.h"

namespace Syn::Vk {
    Image::Image(const ImageConfig& config)
        : _config(config)
    {
        ImageFactory::Allocate(this);
    }

    Image::Image(VkImage existingHandle, const ImageConfig& config)
        : _config(config), _handle(existingHandle)
    {
        _currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        _currentStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        _currentAccess = 0;

        ImageFactory::CreateViews(this);
    }

    Image::~Image() {
        auto device = ServiceLocator::GetVkContext()->GetDevice();

        for (auto& [name, view] : _imageViews) {
            if (view != VK_NULL_HANDLE) {
                vkDestroyImageView(device->Handle(), view, nullptr);
            }
        }

        _imageViews.clear();

        if (_handle != VK_NULL_HANDLE && _allocator != VK_NULL_HANDLE) {
            vmaDestroyImage(_allocator, _handle, _allocation);
        }
    }

    VkImageView Image::GetView(const std::string& name) const {
        if (name.empty()) {
            if (_imageViews.contains("_default"))
                return _imageViews.at("_default");
            return VK_NULL_HANDLE;
        }

        auto it = _imageViews.find(name);
        if (it != _imageViews.end()) {
            return it->second;
        }
        return VK_NULL_HANDLE;
    }

    void Image::TransitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout, VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess, bool discardContent) {
        LayoutTransitionInfo info{};
        info.image = _handle;
        info.aspectMask = _config.aspectMask;

        if (discardContent || _currentLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
            info.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            info.srcStage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            info.srcAccess = 0;
        }
        else {
            info.oldLayout = _currentLayout;
            info.srcStage = _currentStage;
            info.srcAccess = _currentAccess;
        }

        info.newLayout = newLayout;
        info.dstStage = dstStage;
        info.dstAccess = dstAccess;

        ImageUtils::TransitionLayout(cmd, info);

        _currentLayout = newLayout;
        _currentStage = dstStage;
        _currentAccess = dstAccess;
    }
}