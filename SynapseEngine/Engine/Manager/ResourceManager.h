#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/ShaderManager.h"
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
        std::unique_ptr<ShaderManager> _shaderManager;
    };
}