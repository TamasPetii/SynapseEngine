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
	const auto& GetIndirectDrawBuffer(uint32_t frameIndex) const { return indirectCommandBuffers[frameIndex]; }
	const auto& GetInstanceIndexAddressBuffer(uint32_t frameIndex) const { return instanceIndexAddressBuffers[frameIndex]; }
	const auto& GetInstanceIndexBuffer(uint32_t frameIndex, uint32_t bufferIndex) const { return instanceIndexBuffers[frameIndex][bufferIndex]->object; }
	const auto& GetIndirectCountBuffers(uint32_t frameIndex) const { return indirectCountBuffers[frameIndex]; }
protected:
	//Todo: 1 count buffer with uint32_t offsets!
	std::array<std::shared_ptr<Vk::Buffer>, GlobalConfig::FrameConfig::maxFramesInFlights> indirectCountBuffers;
    std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> indirectCommandBuffers;
	std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> instanceIndexAddressBuffers;
	std::array<std::vector<std::shared_ptr<VersionedObject<DynamicSizeBuffer>>>, GlobalConfig::FrameConfig::maxFramesInFlights> instanceIndexBuffers; //VersionedObject to sync instanceIndexAddress update with instance buffers... | Instance version is to sync with model size...
};