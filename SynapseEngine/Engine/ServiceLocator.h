#pragma once
#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"

#include <chrono>
#include <taskflow/taskflow.hpp>
#include <functional>
#include <memory>

namespace Syn::Vk {
    class Context;
    class GpuUploader;
}

namespace Syn {
    struct FrameContext;
    class ShaderManager;
    class ShaderBuilder;
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
    class AudioBuilder;
    class AudioManager;
    class IAudioEngine;
    class IPhysicsEngine;
    class IGpuProfiler;
    class ICpuProfiler;
    class Serializer;
    class IRenderStatCollector;
    class FrameStatisticsManager;
    class PreviewManager;
    class VideoBuilder;
    class VideoManager;

    using PhysicsFactory = std::function<std::unique_ptr<IPhysicsEngine>()>;

    template <typename T>
    struct ServiceTrait {
        using ProvideType = T*;
        using ReturnType = T*;
    };

    template <>
    struct ServiceTrait<PhysicsFactory> {
        using ProvideType = PhysicsFactory;
        using ReturnType = PhysicsFactory&;
    };
}

namespace Syn
{
    class SYN_API ServiceLocator {
    public:
        ServiceLocator() = delete;
        ServiceLocator(const ServiceLocator&) = delete;
        ServiceLocator& operator=(const ServiceLocator&) = delete;

        template <typename T>
        static void Provide(typename ServiceTrait<T>::ProvideType service);

        template <typename T>
        static typename ServiceTrait<T>::ReturnType Get();
    private:
        static Vk::Context* _vkContext;
        static Vk::GpuUploader* _gpuUploader;
        static ShaderManager* _shaderManager;
        static ShaderBuilder* _shaderBuilder;
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
        static AudioBuilder* _audioBuilder;
        static AudioManager* _audioManager;
        static IAudioEngine* _audioEngine;
        static IGpuProfiler* _gpuProfiler;
        static ICpuProfiler* _cpuProfiler;
        static Serializer* _serializer;
        static PhysicsFactory _physicsFactory;
        static IRenderStatCollector* _renderStatCollector;
        static FrameStatisticsManager* _frameStatisticsManager;
        static PreviewManager* _previewManager;
        static VideoBuilder* _videoBuilder;
        static VideoManager* _videoManager;
    };

    template <> SYN_API void ServiceLocator::Provide<Vk::Context>(Vk::Context*);
    template <> SYN_API void ServiceLocator::Provide<Vk::GpuUploader>(Vk::GpuUploader*);
    template <> SYN_API void ServiceLocator::Provide<ShaderManager>(ShaderManager*);
    template <> SYN_API void ServiceLocator::Provide<ShaderBuilder>(ShaderBuilder*);
    template <> SYN_API void ServiceLocator::Provide<ResourceManager>(ResourceManager*);
    template <> SYN_API void ServiceLocator::Provide<StaticMeshBuilder>(StaticMeshBuilder*);
    template <> SYN_API void ServiceLocator::Provide<FrameContext>(FrameContext*);
    template <> SYN_API void ServiceLocator::Provide<ModelManager>(ModelManager*);
    template <> SYN_API void ServiceLocator::Provide<tf::Executor>(tf::Executor*);
    template <> SYN_API void ServiceLocator::Provide<ImageManager>(ImageManager*);
    template <> SYN_API void ServiceLocator::Provide<ImageBuilder>(ImageBuilder*);
    template <> SYN_API void ServiceLocator::Provide<InputManager>(InputManager*);
    template <> SYN_API void ServiceLocator::Provide<SceneManager>(SceneManager*);
    template <> SYN_API void ServiceLocator::Provide<MaterialManager>(MaterialManager*);
    template <> SYN_API void ServiceLocator::Provide<AnimationBuilder>(AnimationBuilder*);
    template <> SYN_API void ServiceLocator::Provide<AnimationManager>(AnimationManager*);
    template <> SYN_API void ServiceLocator::Provide<AudioBuilder>(AudioBuilder*);
    template <> SYN_API void ServiceLocator::Provide<AudioManager>(AudioManager*);
    template <> SYN_API void ServiceLocator::Provide<IAudioEngine>(IAudioEngine*);
    template <> SYN_API void ServiceLocator::Provide<IGpuProfiler>(IGpuProfiler*);
    template <> SYN_API void ServiceLocator::Provide<ICpuProfiler>(ICpuProfiler*);
    template <> SYN_API void ServiceLocator::Provide<Serializer>(Serializer*);
    template <> SYN_API void ServiceLocator::Provide<IRenderStatCollector>(IRenderStatCollector*);
    template <> SYN_API void ServiceLocator::Provide<FrameStatisticsManager>(FrameStatisticsManager*);
    template <> SYN_API void ServiceLocator::Provide<PreviewManager>(PreviewManager*);
    template <> SYN_API void ServiceLocator::Provide<PhysicsFactory>(PhysicsFactory);
    template <> SYN_API void ServiceLocator::Provide<VideoBuilder>(VideoBuilder*);
    template <> SYN_API void ServiceLocator::Provide<VideoManager>(VideoManager*);

    template <> SYN_API Vk::Context* ServiceLocator::Get<Vk::Context>();
    template <> SYN_API Vk::GpuUploader* ServiceLocator::Get<Vk::GpuUploader>();
    template <> SYN_API ShaderManager* ServiceLocator::Get<ShaderManager>();
    template <> SYN_API ShaderBuilder* ServiceLocator::Get<ShaderBuilder>();
    template <> SYN_API ResourceManager* ServiceLocator::Get<ResourceManager>();
    template <> SYN_API StaticMeshBuilder* ServiceLocator::Get<StaticMeshBuilder>();
    template <> SYN_API FrameContext* ServiceLocator::Get<FrameContext>();
    template <> SYN_API ModelManager* ServiceLocator::Get<ModelManager>();
    template <> SYN_API tf::Executor* ServiceLocator::Get<tf::Executor>();
    template <> SYN_API ImageManager* ServiceLocator::Get<ImageManager>();
    template <> SYN_API ImageBuilder* ServiceLocator::Get<ImageBuilder>();
    template <> SYN_API InputManager* ServiceLocator::Get<InputManager>();
    template <> SYN_API SceneManager* ServiceLocator::Get<SceneManager>();
    template <> SYN_API MaterialManager* ServiceLocator::Get<MaterialManager>();
    template <> SYN_API AnimationBuilder* ServiceLocator::Get<AnimationBuilder>();
    template <> SYN_API AnimationManager* ServiceLocator::Get<AnimationManager>();
    template <> SYN_API AudioBuilder* ServiceLocator::Get<AudioBuilder>();
    template <> SYN_API AudioManager* ServiceLocator::Get<AudioManager>();
    template <> SYN_API IAudioEngine* ServiceLocator::Get<IAudioEngine>();
    template <> SYN_API IGpuProfiler* ServiceLocator::Get<IGpuProfiler>();
    template <> SYN_API ICpuProfiler* ServiceLocator::Get<ICpuProfiler>();
    template <> SYN_API Serializer* ServiceLocator::Get<Serializer>();
    template <> SYN_API IRenderStatCollector* ServiceLocator::Get<IRenderStatCollector>();
    template <> SYN_API FrameStatisticsManager* ServiceLocator::Get<FrameStatisticsManager>();
    template <> SYN_API PreviewManager* ServiceLocator::Get<PreviewManager>();
    template <> SYN_API PhysicsFactory& ServiceLocator::Get<PhysicsFactory>();
    template <> SYN_API VideoBuilder* ServiceLocator::Get<VideoBuilder>();
    template <> SYN_API VideoManager* ServiceLocator::Get<VideoManager>();
}