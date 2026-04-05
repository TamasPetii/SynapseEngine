#include "ServiceLocator.h"

namespace Syn {
    Vk::Context* ServiceLocator::_vkContext = nullptr;
	ShaderManager* ServiceLocator::_shaderManager = nullptr;
	ResourceManager* ServiceLocator::_resourceManager = nullptr;
	StaticMeshBuilder* ServiceLocator::_staticMeshBuilder = nullptr;
	FrameContext* ServiceLocator::_frameContext = nullptr;
	ModelManager* ServiceLocator::_modelManager = nullptr;
	tf::Executor* ServiceLocator::_taskExecutor = nullptr;
	ImageManager* ServiceLocator::_imageManager = nullptr;
	ImageBuilder* ServiceLocator::_imageBuilder = nullptr;
	Vk::GpuUploader* ServiceLocator::_gpuUploader = nullptr;
	InputManager* ServiceLocator::_inputManager = nullptr;
	SceneManager* ServiceLocator::_sceneManager = nullptr;
	MaterialManager* ServiceLocator::_materialManager = nullptr;
	AnimationBuilder* ServiceLocator::_animationBuilder = nullptr;
	AnimationManager* ServiceLocator::_animationManager = nullptr;
	IPhysicsEngine* ServiceLocator::_physicsEngine = nullptr;
	IGpuProfiler* ServiceLocator::_gpuProfiler = nullptr;
	ICpuProfiler* ServiceLocator::_cpuProfiler = nullptr;

	void ServiceLocator::Shutdown()
	{
		_vkContext = nullptr;
		_shaderManager = nullptr;
		_resourceManager = nullptr;
		_staticMeshBuilder = nullptr;
		_frameContext = nullptr;
		_modelManager = nullptr;
		_taskExecutor = nullptr;
		_imageManager = nullptr;
		_imageBuilder = nullptr;
		_gpuUploader = nullptr;
		_inputManager = nullptr;
		_sceneManager = nullptr;
		_materialManager = nullptr;
		_animationBuilder = nullptr;
		_animationManager = nullptr;
		_physicsEngine = nullptr;
		_gpuProfiler = nullptr;
		_cpuProfiler = nullptr;
	}
}