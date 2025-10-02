#pragma once
#include "System.h"

struct PointLightComponent;
struct SpotLightComponent;
struct CameraComponent;

class ENGINE_API InstanceSystem : public System
{
public:
	virtual void OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime) override;
	virtual void OnFinish(std::shared_ptr<Registry> registry) override;
	virtual void OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex) override;
private:
	void UpdateShapeInstances(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager);
	void UpdateShapeInstancesGpu(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
	void UpdateShapeInstancesGpuNew(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
	void UpdateShapeInstancesGpuIndirectDraw(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);

	void UpdateModelInstances(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager);
	void UpdateModelInstancesGpu(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
	void UpdateModelInstancesGpuNew(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
	void UpdateModelInstancesGpuIndirectDraw(std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
	
	void UpdatePointLightInstancesGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
	void UpdateSpotLightInstancesGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex);
private:
	static bool IsCameraInsidePointLightSphereVolume(const PointLightComponent& pointLightComponent, const CameraComponent& cameraComponent);
	static bool IsCameraInsidePointLightCubeVolume(const PointLightComponent& pointLightComponent, const CameraComponent& cameraComponent);
	static bool IsCameraInsideSpotLightConeVolume(const SpotLightComponent& spotLightComponent, const CameraComponent& cameraComponent);
};

