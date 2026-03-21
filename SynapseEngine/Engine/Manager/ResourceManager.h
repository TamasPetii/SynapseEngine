#pragma once
#include "Engine/SynApi.h"
#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Animation/Builder/AnimationBuilder.h"
#include "Engine/Animation/AnimationManager.h"
#include <memory>

namespace Syn {

    class SYN_API ResourceManager {
    public:
        ResourceManager();
        ~ResourceManager();

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
    private:
        void InitShaderManager();
        void InitModelManager();
        void InitImageManager();
        void InitMaterialManager();
        void InitAnimationManager();
    private:
        std::unique_ptr<ShaderManager> _shaderManager;
        std::unique_ptr<MaterialManager> _materialManager;

		std::shared_ptr<StaticMeshBuilder> _staticMeshBuilder;
		std::unique_ptr<ModelManager> _modelManager;

        std::shared_ptr<ImageBuilder> _imageBuilder;
        std::unique_ptr<ImageManager> _imageManager;

        std::shared_ptr<AnimationBuilder> _animationBuilder;
        std::unique_ptr<AnimationManager> _animationManager;
    };
}