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

		_staticMeshBuilder->RegisterLoader(std::make_shared<AssimpLoader>(), 1);
		_staticMeshBuilder->RegisterProcessor(std::make_unique<NormalProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<TangentProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<ColliderProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<MeshoptimizerLodProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<MeshoptimizerMeshletProcessor>());

		ServiceLocator::ProvideStaticMeshBuilder(_staticMeshBuilder.get());

		_modelManager = std::make_unique<ModelManager>(
			_staticMeshBuilder,
			std::make_unique<DefaultGpuModelUploader>(),
			[this](const std::string& name, const MaterialInfo& info) -> uint32_t {
				return _materialManager->LoadMaterial(name, info);
			}
		);

		ServiceLocator::ProvideModelManager(_modelManager.get());
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