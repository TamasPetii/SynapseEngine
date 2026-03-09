#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include <taskflow/taskflow.hpp>

namespace Syn::Vk { 
    class Context;
    class GpuUploader;
}

namespace Syn {
    class ShaderManager;
    class ResourceManager;
    class StaticMeshBuilder;
    class ModelManager;
    class ImageManager;
    class ImageBuilder;
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

		static void ProvideTaskExecutor(tf::Executor* executor) { _taskExecutor = executor; }
		static tf::Executor* GetTaskExecutor() { return _taskExecutor; }

		static void ProvideImageManager(ImageManager* manager) { _imageManager = manager; }
		static ImageManager* GetImageManager() { return _imageManager; }

		static void ProvideImageBuilder(ImageBuilder* builder) { _imageBuilder = builder; }
		static ImageBuilder* GetImageBuilder() { return _imageBuilder; }

        static void ProvideGpuUploader(Vk::GpuUploader* uploader) { _gpuUploader = uploader; }
        static Vk::GpuUploader* GetGpuUploader() { return _gpuUploader; }
    private:
        static Vk::Context* _vkContext;
        static Vk::GpuUploader* _gpuUploader;
		static ShaderManager* _shaderManager;
		static ResourceManager* _resourceManager;
		static ModelManager* _modelManager;
        static FrameContext* _frameContext;
		static tf::Executor* _taskExecutor;
		static ImageManager* _imageManager;
		static ImageBuilder* _imageBuilder;
		static StaticMeshBuilder* _staticMeshBuilder;
    };
}