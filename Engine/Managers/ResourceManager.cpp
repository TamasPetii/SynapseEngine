#include "ResourceManager.h"

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
}
