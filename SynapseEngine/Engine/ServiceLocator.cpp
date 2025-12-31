#include "ServiceLocator.h"

namespace Syn {

    Vk::Context* ServiceLocator::_context = nullptr;

    void ServiceLocator::Provide(Vk::Context* context) {
        _context = context;
    }

    void ServiceLocator::Shutdown() {
        _context = nullptr;
    }

    Vk::Context& ServiceLocator::GetContext() {
        SYN_ASSERT(_context != nullptr, "Vulkan Context is not initialized! Ensure Engine::Init() is called before accessing Context.");
        return *_context;
    }

}