#pragma once
#include "Engine/Config.h"
#include "Engine/Vulkan/Buffer.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <array>

class Vk::Buffer;

struct ComponentBuffer
{
	std::shared_ptr<Vk::Buffer> buffer;
	std::vector<uint32_t> versions;
};

//Todo: Refactor

class ENGINE_API ComponentBufferManager
{
public:
	void RegisterBuffer(const std::string& name, const Vk::BufferConfig& config);
	auto* GetComponentBuffer(const std::string& name, uint32_t framesInFlightIndex) { return &(buffers.at(name).second[framesInFlightIndex]); }

	template<typename T>
	void RecreateBuffer(const std::string& name, uint32_t size, uint32_t frameInFlightIndex);
private:
	std::unordered_map<std::string, std::pair<Vk::BufferConfig, std::array<ComponentBuffer, GlobalConfig::FrameConfig::maxFramesInFlights>>> buffers;
};

template<typename T>
inline void ComponentBufferManager::RecreateBuffer(const std::string& name, uint32_t size, uint32_t frameInFlightIndex)
{
	if (buffers.find(name) == buffers.end())
		return;

	auto& [config, buffer] = buffers.at(name);

	uint32_t bufferBaseSize = GlobalConfig::BufferConfig::componentBufferBaseSize;
	uint32_t requiredSize = static_cast<uint32_t>(std::ceil(size / (float)bufferBaseSize)) * bufferBaseSize;
	auto& componentBuffer = buffer.at(frameInFlightIndex);

	if (componentBuffer.versions.size() != requiredSize)
	{
		componentBuffer.versions.clear();
		componentBuffer.versions.resize(requiredSize, 0);

		config.size = requiredSize * sizeof(T);
		componentBuffer.buffer = std::make_shared<Vk::Buffer>(config);
		componentBuffer.buffer->MapMemory();

		std::cout << std::format("Recreated buffer {} with size {}", name, requiredSize) << "\n" << std::endl;
	}
}
