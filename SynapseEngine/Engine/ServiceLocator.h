#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"

namespace Syn::Vk { class Context; }

namespace Syn {

    class SYN_API ServiceLocator {
    public:
        ServiceLocator() = delete;
        ServiceLocator(const ServiceLocator&) = delete;
        ServiceLocator& operator=(const ServiceLocator&) = delete;

        static void ProvideVkContext(Vk::Context* context);
        static void Shutdown();

        static Vk::Context* GetVkContext();
    private:
        static Vk::Context* _vkContext;
    };
}