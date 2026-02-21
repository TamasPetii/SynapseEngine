#include "ServiceLocator.h"

namespace Syn {
    Vk::Context* ServiceLocator::_vkContext = nullptr;
	ShaderManager* ServiceLocator::_shaderManager = nullptr;
	ResourceManager* ServiceLocator::_resourceManager = nullptr;
	StaticMeshBuilder* ServiceLocator::_staticMeshBuilder = nullptr;

	void ServiceLocator::Shutdown()
	{
		_vkContext = nullptr;
		_shaderManager = nullptr;
		_resourceManager = nullptr;
		_staticMeshBuilder = nullptr;
	}
}