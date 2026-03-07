#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"

namespace Syn::Vk { 
    class Context;
}

namespace Syn {
    class ShaderManager;
    class ResourceManager;
    class StaticMeshBuilder;
    class ModelManager;
	struct FrameContext;
}

namespace Syn 
{
    class SYN_API ServiceLocator {
    public:
        ServiceLocator() = delete;
        ServiceLocator(const ServiceLocator&) = delete;
        ServiceLocator& operator=(const ServiceLocator&) = delete;

        static void Shutdown();

        static void ProvideVkContext(Vk::Context* context) { _vkContext = context; }
        static Vk::Context* GetVkContext() { return _vkContext; }

        static void ProvideResourceManager(ResourceManager* manager) { _resourceManager = manager; }
        static ResourceManager* GetResourceManager() { return _resourceManager; }

        static void ProvideShaderManager(ShaderManager* manager) { _shaderManager = manager; }
        static ShaderManager* GetShaderManager() { return _shaderManager; }

        static void ProvideStaticMeshBuilder(StaticMeshBuilder* builder) { _staticMeshBuilder = builder; }
        static StaticMeshBuilder* GetStaticMeshBuilder() { return _staticMeshBuilder; }

        static void ProvideFrameContext(FrameContext* context) { _frameContext = context; }
        static const FrameContext* GetFrameContext() { return _frameContext; }

		static void ProvideModelManager(ModelManager* manager) { _modelManager = manager; }
		static ModelManager* GetModelManager() { return _modelManager; }
    private:
        static Vk::Context* _vkContext;
		static ShaderManager* _shaderManager;
		static ResourceManager* _resourceManager;
		static StaticMeshBuilder* _staticMeshBuilder;
		static ModelManager* _modelManager;
        static FrameContext* _frameContext;
    };
}