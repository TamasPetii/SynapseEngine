#pragma once
#include "System.h"
#include "Engine/Components/TransformComponent.h"
#include "Engine/Components/CameraComponent.h"

class ENGINE_API CameraSystem : public System
{
public:
	virtual void OnUpdate(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex, float deltaTime) override;
	virtual void OnFinish(std::shared_ptr<Registry> registry) override;
	virtual void OnUploadToGpu(std::shared_ptr<Registry> registry, std::shared_ptr<ResourceManager> resourceManager, uint32_t frameIndex) override;
	static Entity GetMainCameraEntity(std::shared_ptr<Registry> registry);
	static float ConvertDepthToLinearNormalized(float depth, float nearPlane, float farPlane);
};

