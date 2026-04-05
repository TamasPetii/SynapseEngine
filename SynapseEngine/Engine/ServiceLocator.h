#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"
#include <taskflow/taskflow.hpp>

namespace Syn::Vk { 
    class Context;
    class GpuUploader;
}

namespace Syn {
	struct FrameContext;
    class ShaderManager;
    class ResourceManager;
    class StaticMeshBuilder;
    class ModelManager;
    class ImageManager;
    class ImageBuilder;
    class InputManager;
    class SceneManager;
    class MaterialManager;
    class AnimationManager;
    class AnimationBuilder;
    class IPhysicsEngine;
	class IGpuProfiler;
    class ICpuProfiler;
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
        static FrameContext* GetFrameContext() { return _frameContext; }

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

        static void ProvideInputManager(InputManager* manager) { _inputManager = manager; }
        static InputManager* GetInputManager() { return _inputManager; }

        static void ProvideSceneManager(SceneManager* manager) { _sceneManager = manager; }
        static SceneManager* GetSceneManager() { return _sceneManager; }

        static void ProvideMaterialManager(MaterialManager* manager) { _materialManager = manager; }
        static MaterialManager* GetMaterialManager() { return _materialManager; }

        static void ProvideAnimationBuilder(AnimationBuilder* builder) { _animationBuilder = builder; }
        static AnimationBuilder* GetAnimationBuilder() { return _animationBuilder; }

        static void ProvideAnimationManager(AnimationManager* manager) { _animationManager = manager; }
        static AnimationManager* GetAnimationManager() { return _animationManager; }

        static void ProvidePhysicsEngine(IPhysicsEngine* engine) { _physicsEngine = engine; }
        static IPhysicsEngine* GetPhysicsEngine() { return _physicsEngine; }

		static void ProvideGpuProfiler(IGpuProfiler* profiler) { _gpuProfiler = profiler; }
		static IGpuProfiler* GetGpuProfiler() { return _gpuProfiler; }

		static void ProvideCpuProfiler(ICpuProfiler* profiler) { _cpuProfiler = profiler; }
		static ICpuProfiler* GetCpuProfiler() { return _cpuProfiler; }
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
        static InputManager* _inputManager;
        static SceneManager* _sceneManager;
        static MaterialManager* _materialManager;
        static AnimationBuilder* _animationBuilder;
        static AnimationManager* _animationManager;
        static IPhysicsEngine* _physicsEngine;
		static IGpuProfiler* _gpuProfiler;
		static ICpuProfiler* _cpuProfiler;
    };
}