#include "ResourceManager.h"
#include "Engine/Components/PointLightComponent.h"
#include "Engine/Components/SpotLightComponent.h"

ResourceManager::ResourceManager()
{
	Initialize();
}

ResourceManager::~ResourceManager()
{
	Cleanup();
}

void ResourceManager::Initialize()
{
	vulkanManager = std::make_shared<VulkanManager>();
	geometryManager = std::make_shared<GeometryManager>();
	componentBufferManager = std::make_shared<ComponentBufferManager>();
	imageManager = std::make_shared<ImageManager>(vulkanManager);
	materialManager = std::make_shared<MaterialManager>(imageManager);
	modelManager = std::make_shared<ModelManager>(imageManager, materialManager);
	benchmarkManager = std::make_shared<BenchmarkManager>();
	animationManager = std::make_shared<AnimationManager>();
	pointLightBufferManager = std::make_shared<LightBufferManager<PointLightComponent>>();
	spotLightBufferManager = std::make_shared<LightBufferManager<SpotLightComponent>>();
}

void ResourceManager::Cleanup()
{
	modelManager.reset();
	imageManager.reset();
	geometryManager.reset();
	componentBufferManager.reset();
	vulkanManager.reset();
	benchmarkManager.reset();
	animationManager.reset();
	materialManager.reset();
	pointLightBufferManager.reset();
	spotLightBufferManager.reset();
}
