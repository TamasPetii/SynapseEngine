#include "ResourceManager.h"
#include "Engine/ServiceLocator.h"

#include "Engine/Manager/ShaderManager.h"
#include "Engine/Mesh/Builder/StaticMeshBuilder.h"
#include "Engine/Mesh/Uploader/DefaultGpuModelUploader.h"
#include "Engine/Mesh/Converter/DefaultModelCooker.h"
#include "Engine/Mesh/Converter/DefaultGpuModelConverter.h"
#include "Engine/Mesh/Converter/DefaultCpuModelExtractor.h"
#include "Engine/Animation/Converter/DefaultCpuAnimationExtractor.h"
#include "Engine/Image/Converter/DefaultCpuImageExtractor.h"

#include "Engine/Mesh/Loader/MeshLoaders.h"
#include "Engine/Mesh/Source/MeshSources.h"
#include "Engine/Mesh/Factory/MeshFactory.h"
#include "Engine/Mesh/Processor/MeshProcessor/MeshProcessors.h" 

#include "Engine/Image/Loader/ImageLoaderRegistry.h"
#include "Engine/Image/Source/Memory/MemoryImageSource.h"
#include "Engine/Image/Processor/ImageProcessorPipeline.h"
#include "Engine/Image/Converter/DefaultGpuImageConverter.h"
#include "Engine/Image/Converter/DefaultImageCooker.h"
#include "Engine/Image/Loader/StbImageLoader.h"
#include "Engine/Image/Loader/GliImageLoader.h"
#include "Engine/Image/Loader/SvgImageLoader.h"
#include "Engine/Image/Loader/HdriImageLoader.h"
#include "Engine/Image/Uploader/DefaultGpuImageUploader.h"

#include "Engine/Animation/Loader/AnimationLoaderRegistry.h"
#include "Engine/Animation/Processor/AnimationProcessorPipeline.h"
#include "Engine/Animation/Converter/DefaultGpuAnimationConverter.h"
#include "Engine/Animation/Converter/DefaultAnimationCooker.h"
#include "Engine/Animation/Loader/AssimpAnimationLoader.h"
#include "Engine/Animation/Processor/Geometry/AnimationBakeProcessor.h"
#include "Engine/Animation/Processor/Geometry/AnimationColliderProcessor.h"
#include "Engine/Animation/Uploader/DefaultGpuAnimationUploader.h"

#include "Engine/Mesh/Processor/CpuModelProcessor/CpuModelProcessorPipeline.h"
#include "Engine/Mesh/Processor/CpuModelProcessor/BatchedIndicesProcessor.h"
#include "Engine/Mesh/Processor/CpuModelProcessor/VertexWeldingProcessor.h"
#include "Engine/Mesh/Processor/CpuModelProcessor/MemoryCleanupProcessor.h"

#include "Engine/Mesh/MeshSourceNames.h"

namespace Syn {

    ResourceManager::ResourceManager(uint32_t framesInFlight) : _framesInFlight(framesInFlight) {
		InitShaderManager();
		InitPreviewManager();
		InitImageManager();
		InitMaterialManager();
		InitModelManager();
		InitAnimationManager();
    }

	void ResourceManager::InitPreviewManager() {
		_previewManager = std::make_unique<PreviewManager>(2048, 128);
		ServiceLocator::ProvidePreviewManager(_previewManager.get());
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
		_imageBuilder->RegisterLoader(std::make_shared<SvgImageLoader>(), 1);
		_imageBuilder->RegisterLoader(std::make_shared<HdriImageLoader>(), 1);

		ServiceLocator::ProvideImageBuilder(_imageBuilder.get());

		_imageManager = std::make_unique<ImageManager>(
			_framesInFlight,
			_imageBuilder,
			std::make_unique<DefaultGpuImageUploader>(),
			std::make_unique<DefaultCpuImageExtractor>()
		);

		ServiceLocator::ProvideImageManager(_imageManager.get());
	}

	void ResourceManager::InitMaterialManager()
	{
		_materialManager = std::make_unique<MaterialManager>(
			_framesInFlight,
			[this](const TexturePayload& payload) -> uint32_t {
				if (payload.IsEmbedded()) {
					size_t hash = payload.embeddedData.size();

					if (hash > 0) {
						hash ^= (payload.embeddedData.front() << 16) | payload.embeddedData.back();
					}

					std::string uniqueName = "Embedded_" + std::to_string(hash) + "_" + payload.path;

					std::string ext = payload.formatHint.empty() ? "" : "." + payload.formatHint;
					IImageLoader* loader = _imageBuilder->GetLoaderForExtension(ext);

					return _imageManager->LoadImageFromSourceAsync(uniqueName, [payload, loader]() {
						return std::make_unique<MemoryImageSource>(payload, loader);
						});
				}
				else {
					return _imageManager->LoadImageAsync(payload.path);
				}
			},
			[this](uint32_t id) {
				if (_previewManager) _previewManager->AllocateTile(PreviewResourceType::Material, id);
			},
			[this](uint32_t id) {
				if (_previewManager) _previewManager->MarkDirty(PreviewResourceType::Material, id);
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
			std::make_unique<DefaultModelCooker>(),
			std::make_unique<DefaultCpuModelExtractor>(),
			std::make_unique<CpuModelProcessorPipeline>()
		);

		_staticMeshBuilder->RegisterLoader(std::make_shared<AssimpMeshLoader>(), 1);

		_staticMeshBuilder->RegisterMeshProcessor(std::make_unique<NormalProcessor>());
		_staticMeshBuilder->RegisterMeshProcessor(std::make_unique<TangentProcessor>());
		_staticMeshBuilder->RegisterMeshProcessor(std::make_unique<MeshoptimizerLodProcessor>());
		//_staticMeshBuilder->RegisterProcessor(std::make_unique<MeshoptimizerOptimizerProcessor>());
		_staticMeshBuilder->RegisterMeshProcessor(std::make_unique<MeshoptimizerMeshletProcessor>());
		_staticMeshBuilder->RegisterMeshProcessor(std::make_unique<ColliderProcessor>());

		_staticMeshBuilder->RegisterCpuModelProcessor(std::make_unique<BatchedIndicesProcessor>());
		_staticMeshBuilder->RegisterCpuModelProcessor(std::make_unique<VertexWeldingProcessor>());
		_staticMeshBuilder->RegisterCpuModelProcessor(std::make_unique<MemoryCleanupProcessor>());

		ServiceLocator::ProvideStaticMeshBuilder(_staticMeshBuilder.get());

		_modelManager = std::make_unique<ModelManager>(
			_framesInFlight,
			_staticMeshBuilder,
			std::make_unique<DefaultGpuModelUploader>(),
			[this](const std::string& name, const MaterialInfo& info) -> uint32_t {
				return _materialManager->LoadMaterial(name, info);
			},
			[this](uint32_t id) {
				if (_previewManager) _previewManager->AllocateTile(PreviewResourceType::Model, id);
			},
			[this](uint32_t id) {
				if (_previewManager) _previewManager->MarkDirty(PreviewResourceType::Model, id);
			}
		);

		ServiceLocator::ProvideModelManager(_modelManager.get());

		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Sphere, []() { return MeshFactory::CreateSphere(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::ProxySphere, []() { return MeshFactory::CreateProxySphere(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Cube, []() { return MeshFactory::CreateCube(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Quad, []() { return MeshFactory::CreateQuad(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::ScreenQuad, []() { return MeshFactory::CreateScreenQuad(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Cylinder, []() { return MeshFactory::CreateCylinder(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Cone, []() { return MeshFactory::CreateCone(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::ProxyCone, []() { return MeshFactory::CreateProxyCone(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Capsule, []() { return MeshFactory::CreateCapsule(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Hemisphere, []() { return MeshFactory::CreateHemisphere(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Pyramid, []() { return MeshFactory::CreatePyramid(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::ProxyPyramid, []() { return MeshFactory::CreateProxyPyramid(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Grid, []() { return MeshFactory::CreateGrid(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::Torus, []() { return MeshFactory::CreateTorus(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::IcoSphere, []() { return MeshFactory::CreateIcoSphere(); });
		_modelManager->LoadModelFromStaticMeshSync(MeshSourceNames::ProxyIcoSphere, []() { return MeshFactory::CreateProxyIcoSphere(); });

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
			_framesInFlight,
			_animationBuilder,
			std::make_unique<DefaultGpuAnimationUploader>(),
			std::make_unique<DefaultCpuAnimationExtractor>()
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