#include "ServiceLocator.h"

namespace Syn
{
    Vk::Context* ServiceLocator::_vkContext = nullptr;
    Vk::GpuUploader* ServiceLocator::_gpuUploader = nullptr;
    ShaderManager* ServiceLocator::_shaderManager = nullptr;
    ResourceManager* ServiceLocator::_resourceManager = nullptr;
    StaticMeshBuilder* ServiceLocator::_staticMeshBuilder = nullptr;
    FrameContext* ServiceLocator::_frameContext = nullptr;
    ModelManager* ServiceLocator::_modelManager = nullptr;
    tf::Executor* ServiceLocator::_taskExecutor = nullptr;
    ImageManager* ServiceLocator::_imageManager = nullptr;
    ImageBuilder* ServiceLocator::_imageBuilder = nullptr;
    InputManager* ServiceLocator::_inputManager = nullptr;
    SceneManager* ServiceLocator::_sceneManager = nullptr;
    MaterialManager* ServiceLocator::_materialManager = nullptr;
    AnimationBuilder* ServiceLocator::_animationBuilder = nullptr;
    AnimationManager* ServiceLocator::_animationManager = nullptr;
    AudioBuilder* ServiceLocator::_audioBuilder = nullptr;
    AudioManager* ServiceLocator::_audioManager = nullptr;
    IAudioEngine* ServiceLocator::_audioEngine = nullptr;
    IGpuProfiler* ServiceLocator::_gpuProfiler = nullptr;
    ICpuProfiler* ServiceLocator::_cpuProfiler = nullptr;
    Serializer* ServiceLocator::_serializer = nullptr;
    PhysicsFactory ServiceLocator::_physicsFactory = nullptr;
    IRenderStatCollector* ServiceLocator::_renderStatCollector = nullptr;
    FrameStatisticsManager* ServiceLocator::_frameStatisticsManager = nullptr;
    PreviewManager* ServiceLocator::_previewManager = nullptr;

    template <> void ServiceLocator::Provide<Vk::Context>(Vk::Context* s) { _vkContext = s; }
    template <> void ServiceLocator::Provide<Vk::GpuUploader>(Vk::GpuUploader* s) { _gpuUploader = s; }
    template <> void ServiceLocator::Provide<ShaderManager>(ShaderManager* s) { _shaderManager = s; }
    template <> void ServiceLocator::Provide<ResourceManager>(ResourceManager* s) { _resourceManager = s; }
    template <> void ServiceLocator::Provide<StaticMeshBuilder>(StaticMeshBuilder* s) { _staticMeshBuilder = s; }
    template <> void ServiceLocator::Provide<FrameContext>(FrameContext* s) { _frameContext = s; }
    template <> void ServiceLocator::Provide<ModelManager>(ModelManager* s) { _modelManager = s; }
    template <> void ServiceLocator::Provide<tf::Executor>(tf::Executor* s) { _taskExecutor = s; }
    template <> void ServiceLocator::Provide<ImageManager>(ImageManager* s) { _imageManager = s; }
    template <> void ServiceLocator::Provide<ImageBuilder>(ImageBuilder* s) { _imageBuilder = s; }
    template <> void ServiceLocator::Provide<InputManager>(InputManager* s) { _inputManager = s; }
    template <> void ServiceLocator::Provide<SceneManager>(SceneManager* s) { _sceneManager = s; }
    template <> void ServiceLocator::Provide<MaterialManager>(MaterialManager* s) { _materialManager = s; }
    template <> void ServiceLocator::Provide<AnimationBuilder>(AnimationBuilder* s) { _animationBuilder = s; }
    template <> void ServiceLocator::Provide<AnimationManager>(AnimationManager* s) { _animationManager = s; }
    template <> void ServiceLocator::Provide<AudioBuilder>(AudioBuilder* s) { _audioBuilder = s; }
    template <> void ServiceLocator::Provide<AudioManager>(AudioManager* s) { _audioManager = s; }
    template <> void ServiceLocator::Provide<IAudioEngine>(IAudioEngine* s) { _audioEngine = s; }
    template <> void ServiceLocator::Provide<IGpuProfiler>(IGpuProfiler* s) { _gpuProfiler = s; }
    template <> void ServiceLocator::Provide<ICpuProfiler>(ICpuProfiler* s) { _cpuProfiler = s; }
    template <> void ServiceLocator::Provide<Serializer>(Serializer* s) { _serializer = s; }
    template <> void ServiceLocator::Provide<IRenderStatCollector>(IRenderStatCollector* s) { _renderStatCollector = s; }
    template <> void ServiceLocator::Provide<FrameStatisticsManager>(FrameStatisticsManager* s) { _frameStatisticsManager = s; }
    template <> void ServiceLocator::Provide<PreviewManager>(PreviewManager* s) { _previewManager = s; }
    template <> void ServiceLocator::Provide<PhysicsFactory>(PhysicsFactory f) { _physicsFactory = std::move(f); }

    template <> Vk::Context* ServiceLocator::Get<Vk::Context>() { return _vkContext; }
    template <> Vk::GpuUploader* ServiceLocator::Get<Vk::GpuUploader>() { return _gpuUploader; }
    template <> ShaderManager* ServiceLocator::Get<ShaderManager>() { return _shaderManager; }
    template <> ResourceManager* ServiceLocator::Get<ResourceManager>() { return _resourceManager; }
    template <> StaticMeshBuilder* ServiceLocator::Get<StaticMeshBuilder>() { return _staticMeshBuilder; }
    template <> FrameContext* ServiceLocator::Get<FrameContext>() { return _frameContext; }
    template <> ModelManager* ServiceLocator::Get<ModelManager>() { return _modelManager; }
    template <> tf::Executor* ServiceLocator::Get<tf::Executor>() { return _taskExecutor; }
    template <> ImageManager* ServiceLocator::Get<ImageManager>() { return _imageManager; }
    template <> ImageBuilder* ServiceLocator::Get<ImageBuilder>() { return _imageBuilder; }
    template <> InputManager* ServiceLocator::Get<InputManager>() { return _inputManager; }
    template <> SceneManager* ServiceLocator::Get<SceneManager>() { return _sceneManager; }
    template <> MaterialManager* ServiceLocator::Get<MaterialManager>() { return _materialManager; }
    template <> AnimationBuilder* ServiceLocator::Get<AnimationBuilder>() { return _animationBuilder; }
    template <> AnimationManager* ServiceLocator::Get<AnimationManager>() { return _animationManager; }
    template <> AudioBuilder* ServiceLocator::Get<AudioBuilder>() { return _audioBuilder; }
    template <> AudioManager* ServiceLocator::Get<AudioManager>() { return _audioManager; }
    template <> IAudioEngine* ServiceLocator::Get<IAudioEngine>() { return _audioEngine; }
    template <> IGpuProfiler* ServiceLocator::Get<IGpuProfiler>() { return _gpuProfiler; }
    template <> ICpuProfiler* ServiceLocator::Get<ICpuProfiler>() { return _cpuProfiler; }
    template <> Serializer* ServiceLocator::Get<Serializer>() { return _serializer; }
    template <> IRenderStatCollector* ServiceLocator::Get<IRenderStatCollector>() { return _renderStatCollector; }
    template <> FrameStatisticsManager* ServiceLocator::Get<FrameStatisticsManager>() { return _frameStatisticsManager; }
    template <> PreviewManager* ServiceLocator::Get<PreviewManager>() { return _previewManager; }
    template <> PhysicsFactory& ServiceLocator::Get<PhysicsFactory>() { return _physicsFactory; }
}