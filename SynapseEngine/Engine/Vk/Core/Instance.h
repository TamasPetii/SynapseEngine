#pragma once
#include "../VkCommon.h"

namespace Syn::Vk {
    class SYN_API Instance {
    public:
        Instance(bool enableValidation, std::span<const char*> windowExtensions);
        ~Instance();

        VkInstance Handle() const { return _handle; }
        bool IsValidationEnabled() const { return _validationEnabled; }
    private:
        void SetupVolk();
        void CreateInstance(std::span<const char*> extensions);
        void SetupDebugMessenger();
    private:
        VkInstance _handle = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
        bool _validationEnabled = false;
    };
}