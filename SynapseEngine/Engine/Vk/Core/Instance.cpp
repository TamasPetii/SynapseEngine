#include "Instance.h"
#include "Engine/Logger/LogMessage.h"
#include "Engine/Logger/Logger.h"

namespace Syn::Vk {

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        LogLevel level = LogLevel::Info;

        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            level = LogLevel::Error;
        }
        else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            level = LogLevel::Warning;
        }
        else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            level = LogLevel::Info;
        }

        std::string message = std::format("[Vulkan Validation] {}", pCallbackData->pMessage);

        Logger::Get().Dispatch(level, message, "VulkanDriver", 0);

        return VK_FALSE;
    }

    Instance::Instance(bool enableValidation, std::span<const char*> windowExtensions)
        : _validationEnabled(enableValidation)
    {
        SetupVolk();

        CreateInstance(windowExtensions);

        if (_validationEnabled)
            SetupDebugMessenger();
    }

    Instance::~Instance()
    {
        if (_debugMessenger != VK_NULL_HANDLE)
            vkDestroyDebugUtilsMessengerEXT(_handle, _debugMessenger, nullptr);

        if (_handle != VK_NULL_HANDLE)
            vkDestroyInstance(_handle, nullptr);
    }

    void Instance::SetupVolk()
    {
        SYN_VK_ASSERT_MSG(volkInitialize(), "Failed to initialize volk");
    }

    void Instance::CreateInstance(std::span<const char*> windowExtensions)
    {
        VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
        appInfo.pApplicationName = "Synapse Engine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Synapse";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_4;

        std::vector<const char*> extensions(windowExtensions.begin(), windowExtensions.end());
        if (_validationEnabled)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        std::vector<const char*> layers;
        if (_validationEnabled)
            layers.push_back("VK_LAYER_KHRONOS_validation");

        VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        createInfo.ppEnabledLayerNames = layers.data();

        VkDebugUtilsMessengerCreateInfoEXT debugInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        if (_validationEnabled) {
            debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugInfo.pfnUserCallback = DebugCallback;
            createInfo.pNext = &debugInfo;
        }

        SYN_VK_ASSERT_MSG(vkCreateInstance(&createInfo, nullptr, &_handle), "Failed to create Vulkan Instance");
        volkLoadInstance(_handle);
    }

    void Instance::SetupDebugMessenger() {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;

        SYN_VK_ASSERT_MSG(vkCreateDebugUtilsMessengerEXT(_handle, &createInfo, nullptr, &_debugMessenger), "Failed to set up debug messenger");
    }
}