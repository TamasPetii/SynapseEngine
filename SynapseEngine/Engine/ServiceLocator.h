#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"

namespace Syn::Vk { 
    class Context;
}

namespace Syn {
    class ShaderManager;
    class ResourceManager;
}

namespace Syn {

    class SYN_API ServiceLocator {
    public:
        ServiceLocator() = delete;
        ServiceLocator(const ServiceLocator&) = delete;
        ServiceLocator& operator=(const ServiceLocator&) = delete;

        static void Shutdown();

        static void ProvideVkContext(Vk::Context* context) { _vkContext = context; }
        static void ProvideResourceManager(ResourceManager* manager) { _resourceManager = manager; }
        static void ProvideShaderManager(ShaderManager* manager) { _shaderManager = manager; }

        static Vk::Context* GetVkContext() { return _vkContext; }
        static ShaderManager* GetShaderManager() { return _shaderManager; }
        static ResourceManager* GetResourceManager() { return _resourceManager; }
    private:
        static Vk::Context* _vkContext;
		static ShaderManager* _shaderManager;
		static ResourceManager* _resourceManager;
    };
}