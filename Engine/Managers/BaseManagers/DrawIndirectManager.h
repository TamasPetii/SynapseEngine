#pragma once
#include <array>
#include "Engine/EngineApi.h"
#include "ArrayIndexedManager.h"
#include "Engine/Vulkan/Buffer.h"
#include "Engine/Config.h"

#include "DynamicSizeBuffer.h"
#include "Engine/Utils/VersionedObject.h"

class DrawIndirectManager
{
public:
    DrawIndirectManager();
	void Update(uint32_t frameIndex, uint32_t count, uint32_t bufferBaseSize);
	void UpdateInstanceBuffer(uint32_t frameIndex, uint32_t bufferIndex, uint32_t count); //This should be called after Update!!
protected:
    std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> indirectCommandBuffers;
	std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> instanceIndexAddressBuffers;
	std::array<std::vector<std::shared_ptr<VersionedObject<DynamicSizeBuffer>>>, GlobalConfig::FrameConfig::maxFramesInFlights> instanceIndexBuffers; //VersionedObject to sync instanceIndexAddress update with instance buffers... | Instance version is to sync with model size...
};