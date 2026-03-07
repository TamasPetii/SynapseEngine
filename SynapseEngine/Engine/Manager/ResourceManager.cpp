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

namespace Syn {

    ResourceManager::ResourceManager() {
        _shaderManager = std::make_unique<ShaderManager>();
        ServiceLocator::ProvideShaderManager(_shaderManager.get());

		_staticMeshBuilder = std::make_shared<StaticMeshBuilder>(
			std::make_unique<MeshLoaderRegistry>(),
			std::make_unique<MeshProcessorPipeline>(),
			std::make_unique<DefaultGpuModelConverter>(),
			std::make_unique<DefaultModelCooker>()
		);

		_staticMeshBuilder->RegisterLoader(std::make_shared<AssimpLoader>(), 100);
		_staticMeshBuilder->RegisterProcessor(std::make_unique<NormalProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<TangentProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<ColliderProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<MeshoptimizerLodProcessor>());
		_staticMeshBuilder->RegisterProcessor(std::make_unique<MeshoptimizerMeshletProcessor>());

		ServiceLocator::ProvideStaticMeshBuilder(_staticMeshBuilder.get());

        _modelManager = std::make_unique<ModelManager>(
			_staticMeshBuilder,
			std::make_unique<DefaultGpuModelUploader>()
		);

		ServiceLocator::ProvideModelManager(_modelManager.get());
    }

    ResourceManager::~ResourceManager() {
        ServiceLocator::ProvideShaderManager(nullptr);
        ServiceLocator::ProvideResourceManager(nullptr);
		ServiceLocator::ProvideStaticMeshBuilder(nullptr);
		ServiceLocator::ProvideModelManager(nullptr);
    }
}