#pragma once
#include <array>
#include "Engine/EngineApi.h"
#include "ArrayIndexedManager.h"
#include "Engine/Vulkan/Buffer.h"
#include "Engine/Config.h"

struct ENGINE_API DrawBuffers
{
	std::shared_ptr<Vk::Buffer> instanceIndexBuffer;

};

class DrawIndirectManager
{
public:
    DrawIndirectManager();
	void Update(uint32_t frameIndex, uint32_t currentSize);
protected:
    std::array<std::pair<std::shared_ptr<Vk::Buffer>, uint32_t>, GlobalConfig::FrameConfig::maxFramesInFlights> indirectCommandBuffers;
	std::array<std::pair<std::shared_ptr<Vk::Buffer>, uint32_t>, GlobalConfig::FrameConfig::maxFramesInFlights> instanceIndexBuffers;
};