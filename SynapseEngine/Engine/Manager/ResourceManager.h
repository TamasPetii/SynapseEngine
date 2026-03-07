#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Manager/ModelManager.h"
#include "Engine/Manager/ImageManager.h"
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
        void InitShaderManager();
        void InitModelManager();
        void InitImageManager();
    private:
		std::shared_ptr<StaticMeshBuilder> _staticMeshBuilder;
        std::shared_ptr<ImageBuilder> _imageBuilder;

        std::unique_ptr<ShaderManager> _shaderManager;
		std::unique_ptr<ModelManager> _modelManager;
        std::unique_ptr<ImageManager> _imageManager;
    };
}