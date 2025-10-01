#pragma once
#include <array>
#include "Engine/EngineApi.h"
#include "Engine/Vulkan/Buffer.h"
#include "Engine/Config.h"

#include "BaseManagers/DynamicSizeBuffer.h"
#include "Engine/Utils/VersionedObject.h"
#include "Engine/Registry/Registry.h"

class LightBufferManager
{
public:
	LightBufferManager();
	void Update(std::shared_ptr<Registry> registry, uint32_t frameIndex);
private:
	void UpdateBuffer(std::shared_ptr<DynamicSizeBuffer> dynamicBuffer, uint32_t requiredSize);
	void UpdatePointLightResources(std::shared_ptr<Registry> registry, uint32_t frameIndex, uint32_t baseBufferSize);
	void UpdateSpotLightResources(std::shared_ptr<Registry> registry, uint32_t frameIndex, uint32_t baseBufferSize);
private:
	std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> pointLightInstanceIndexBuffers;
	std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> pointLightShadowInstanceIndexBuffers;
	std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> spotLightInstanceIndexBuffers;
	std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> spotLightShadowInstanceIndexBuffers;
};

