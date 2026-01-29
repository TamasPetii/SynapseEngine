#include "ServiceLocator.h"

namespace Syn {

    Vk::Context* ServiceLocator::_vkContext = nullptr;

    void ServiceLocator::ProvideVkContext(Vk::Context* context) {
        _vkContext = context;
    }

    void ServiceLocator::Shutdown() {
        _vkContext = nullptr;
    }

    Vk::Context* ServiceLocator::GetVkContext() {
        SYN_ASSERT(_vkContext != nullptr, "Vulkan Context is not initialized! Ensure Engine::Init() is called before accessing Context.");
        return _vkContext;
    }

}