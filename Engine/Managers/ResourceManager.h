#pragma once
#include "ImageManager.h"
#include "VulkanManager.h"
#include "GeometryManager.h"
#include "ComponentBufferManager.h"
#include "ModelManager.h"
#include "BenchmarkManager.h"
#include "AnimationManager.h"
#include "MaterialManager.h"
#include "LightBufferManager.h"

class ENGINE_API ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();
	void Initialize();
	void Cleanup();

	auto GetModelManager() { return modelManager; }
	auto GetImageManager() { return imageManager; }
	auto GetVulkanManager() { return vulkanManager; }
	auto GetGeometryManager() { return geometryManager; }
	auto GetComponentBufferManager() { return componentBufferManager; }
	auto GetBenchmarkManager() { return benchmarkManager; }
	auto GetAnimationManager() { return animationManager; }
	auto GetMaterialManager() { return materialManager; }
	auto GetLightBufferManager() { return lightBufferManager; }
private:
	std::shared_ptr<ModelManager> modelManager;
	std::shared_ptr<ImageManager> imageManager;
	std::shared_ptr<VulkanManager> vulkanManager;
	std::shared_ptr<GeometryManager> geometryManager;
	std::shared_ptr<ComponentBufferManager> componentBufferManager;
	std::shared_ptr<BenchmarkManager> benchmarkManager;
	std::shared_ptr<AnimationManager> animationManager;
	std::shared_ptr<MaterialManager> materialManager;
	std::shared_ptr<LightBufferManager> lightBufferManager;
};

