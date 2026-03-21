#include "ResourceManager.h"
#include "Engine/ServiceLocator.h"

#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/Builder/StaticMeshBuilder.h"
#include "Engine/Mesh/Uploader/DefaultGpuModelUploader.h"
#include "Engine/Mesh/Converter/DefaultModelCooker.h"
#include "Engine/Mesh/Converter/DefaultGpuModelConverter.h"

#include "Engine/Mesh/Loader/MeshLoaders.h"
#include "Engine/Mesh/Source/MeshSources.h"
#include "Engine/Mesh/Factory/MeshFactory.h"
#include "Engine/Mesh/Processor/MeshProcessors.h" 

#include "Engine/Image/Loader/ImageLoaderRegistry.h"
#include "Engine/Image/Processor/ImageProcessorPipeline.h"
#include "Engine/Image/Converter/DefaultGpuImageConverter.h"
#include "Engine/Image/Converter/DefaultImageCooker.h"
#include "Engine/Image/Loader/StbImageLoader.h"
#include "Engine/Image/Loader/GliImageLoader.h"
#include "Engine/Image/Uploader/DefaultGpuImageUploader.h"

#include "Engine/Animation/Loader/AnimationLoaderRegistry.h"
#include "Engine/Animation/Processor/AnimationProcessorPipeline.h"
#include "Engine/Animation/Converter/DefaultGpuAnimationConverter.h"
#include "Engine/Animation/Converter/DefaultAnimationCooker.h"
#include "Engine/Animation/Loader/AssimpAnimationLoader.h"
#include "Engine/Animation/Processor/Geometry/AnimationBakeProcessor.h"
#include "Engine/Animation/Processor/Geometry/AnimationColliderProcessor.h"
#include "Engine/Animation/Uploader/DefaultGpuAnimationUploader.h"


#include "Engine/Mesh/MeshSourceNames.h"

namespace Syn {

    ResourceManager::ResourceManager() {
		InitShaderManager();
		InitImageManager();
		InitMaterialManager();
		InitModelManager();
    }

	void ResourceManager::InitShaderManager()
	{
		_shaderManager = std::make_unique<ShaderManager>();
		ServiceLocator::ProvideShaderManager(_shaderManager.get());
	}

	void ResourceManager::InitImageManager()
	{
		_imageBuilder = std::make_shared<ImageBuilder>(
			std::make_unique<ImageLoaderRegistry>(),
			std::make_unique<ImageProcessorPipeline>(),
			std::make_unique<DefaultGpuImageConverter>(),
			std::make_unique<DefaultImageCooker>()
		);

		_imageBuilder->RegisterLoader(std::make_shared<StbImageLoader>(), 1);
		_imageBuilder->RegisterLoader(std::make_shared<GliImageLoader>(), 1);

		ServiceLocator::ProvideImageBuilder(_imageBuilder.get());

		_imageManager = std::make_unique<ImageManager>(
			_imageBuilder,
			std::make_unique<DefaultGpuImageUploader>()
		);

		ServiceLocator::ProvideImageManager(_imageManager.get());
	}

	void ResourceManager::InitMaterialManager()
	{
		_materialManager = std::make_unique<MaterialManager>(
			[this](const std::string& fullPath) -> uint32_t {
				return _imageManager->LoadImageAsync(fullPath);
			}
		);

		ServiceLocator::ProvideMaterialManager(_materialManager.get());
	}

	void ResourceManager::InitModelManager()
	{
		_staticMeshBuilder = std::make_shared<StaticMeshBuilder>(
			std::make_unique<MeshLoaderRegistry>(),
			std::make_unique<MeshProcessorPipeline>(),
			std::make_unique<DefaultGpuModelConverter>(),
			std::make_unique<DefaultModelCooker>()
		);

		_staticMeshBuilder->RegisterLoader(std::make_shared<AssimpMeshLoader>(), 1);
		_staticMeshBuilder->RegisterProcessor(std::make_unique<NormalProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<TangentProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<MeshoptimizerLodProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<MeshoptimizerOptimizerProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<MeshoptimizerMeshletProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<ColliderProcessor>());

		ServiceLocator::ProvideStaticMeshBuilder(_staticMeshBuilder.get());

		_modelManager = std::make_unique<ModelManager>(
			_staticMeshBuilder,
			std::make_unique<DefaultGpuModelUploader>(),
			[this](const std::string& name, const MaterialInfo& info) -> uint32_t {
				return _materialManager->LoadMaterial(name, info);
			}
		);

		ServiceLocator::ProvideModelManager(_modelManager.get());

		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Sphere, []() { return MeshFactory::CreateSphere(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Cube, []() { return MeshFactory::CreateCube(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Quad, []() { return MeshFactory::CreateQuad(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::ScreenQuad, []() { return MeshFactory::CreateScreenQuad(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Cylinder, []() { return MeshFactory::CreateCylinder(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Cone, []() { return MeshFactory::CreateCone(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Capsule, []() { return MeshFactory::CreateCapsule(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Hemisphere, []() { return MeshFactory::CreateHemisphere(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Pyramid, []() { return MeshFactory::CreatePyramid(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Grid, []() { return MeshFactory::CreateGrid(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Torus, []() { return MeshFactory::CreateTorus(); });
	}

	void ResourceManager::InitAnimationManager()
	{
		_animationBuilder = std::make_shared<AnimationBuilder>(
			std::make_unique<AnimationLoaderRegistry>(),
			std::make_unique<AnimationProcessorPipeline>(),
			std::make_unique<DefaultGpuAnimationConverter>(),
			std::make_unique<DefaultAnimationCooker>()
		);

		_animationBuilder->RegisterLoader(std::make_shared<AssimpAnimationLoader>(), 1);
		_animationBuilder->RegisterProcessor(std::make_unique<AnimationBakeProcessor>());
		_animationBuilder->RegisterProcessor(std::make_unique<AnimationColliderProcessor>());

		ServiceLocator::ProvideAnimationBuilder(_animationBuilder.get());

		_animationManager = std::make_unique<AnimationManager>(
			_animationBuilder,
			std::make_unique<DefaultGpuAnimationUploader>()
		);

		ServiceLocator::ProvideAnimationManager(_animationManager.get());
	}

    ResourceManager::~ResourceManager() {
        ServiceLocator::ProvideShaderManager(nullptr);
        ServiceLocator::ProvideResourceManager(nullptr);
		ServiceLocator::ProvideStaticMeshBuilder(nullptr);
		ServiceLocator::ProvideModelManager(nullptr);
		ServiceLocator::ProvideImageBuilder(nullptr);
		ServiceLocator::ProvideImageManager(nullptr);
		ServiceLocator::ProvideMaterialManager(nullptr);
    }
}