#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ModelManager.h"
#include <memory>

namespace Syn {

    class SYN_API ResourceManager {
    public:
        ResourceManager();
        ~ResourceManager();

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;

       ShaderManager* GetShaderManager() const { return _shaderManager.get(); }
    private:
		std::shared_ptr<StaticMeshBuilder> _staticMeshBuilder;
        std::unique_ptr<ShaderManager> _shaderManager;
		std::unique_ptr<ModelManager> _modelManager;
    };
}