#pragma once
#include "System.h"

struct FrustumCollider;

class ENGINE_API FrustumCullingSystem : public System
{
public:
	virtual void OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime) override;
	virtual void OnFinish(std::shared_ptr<Registry> registry) override;
	virtual void OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex) override;
private:
	void DefaultColliderCulling(std::shared_ptr<Registry> registry, FrustumCollider* cameraCollider);
	void PointLightCulling(std::shared_ptr<Registry> registry, FrustumCollider* cameraCollider);
	void SpotLightCulling(std::shared_ptr<Registry> registry, FrustumCollider* cameraCollider);
};

