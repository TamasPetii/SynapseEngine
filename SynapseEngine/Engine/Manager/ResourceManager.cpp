#include "ResourceManager.h"
#include "Engine/ServiceLocator.h"

namespace Syn {

    ResourceManager::ResourceManager() {
        _shaderManager = std::make_unique<ShaderManager>();
        ServiceLocator::ProvideShaderManager(_shaderManager.get());
    }

    ResourceManager::~ResourceManager() {
        ServiceLocator::ProvideShaderManager(nullptr);
        ServiceLocator::ProvideResourceManager(nullptr);
    }
}