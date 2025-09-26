#pragma once
#include "Engine/EngineApi.h"
#include "ArrayIndexedManager.h"
#include "DynamicSizeBuffer.h"
#include "Engine/Config.h"
#include <array>

template<typename T>
class DeviceAddressedManager : public ArrayIndexedManager
{
public:
	DeviceAddressedManager();
    virtual ~DeviceAddressedManager();
    void Update(uint32_t frameIndex, uint32_t count, uint32_t baseBufferSize);

	const auto& GetDeviceAddressesBuffer(uint32_t frameIndex) const { return deviceAddressBuffers[frameIndex]; }
protected:
    std::array<std::shared_ptr<DynamicSizeBuffer>, GlobalConfig::FrameConfig::maxFramesInFlights> deviceAddressBuffers;
};

template<typename T>
DeviceAddressedManager<T>::DeviceAddressedManager()
{
    for (uint32_t i = 0; i < deviceAddressBuffers.size(); ++i)
        deviceAddressBuffers[i] = std::make_shared<DynamicSizeBuffer>();
}

template<typename T>
DeviceAddressedManager<T>::~DeviceAddressedManager()
{
    for (auto& dynamicBuffer : deviceAddressBuffers)
    {
        if (dynamicBuffer != nullptr)
        {
            dynamicBuffer.reset();
            dynamicBuffer = nullptr;
        }
    }
}

template<typename T>
void DeviceAddressedManager<T>::Update(uint32_t frameIndex, uint32_t count, uint32_t baseBufferSize)
{
    if (deviceAddressBuffers[frameIndex] == nullptr)
        return; //Should be never!!

    uint32_t requiredSize = static_cast<uint32_t>(std::ceil((count + 1) / (float)baseBufferSize)) * baseBufferSize;

    if (deviceAddressBuffers[frameIndex]->buffer == nullptr || deviceAddressBuffers[frameIndex]->size != requiredSize)
    {
        Vk::BufferConfig config;
        config.size = requiredSize * sizeof(T);
        config.usage = VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
        config.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        deviceAddressBuffers[frameIndex]->size = requiredSize;
        deviceAddressBuffers[frameIndex]->buffer = std::make_shared<Vk::Buffer>(config);
        deviceAddressBuffers[frameIndex]->buffer->MapMemory();
        deviceAddressBuffers[frameIndex]->version++;

        std::cout << std::format("DeviceAddressManager buffer updated for frameIndex {}: New Size = {}", frameIndex, requiredSize) << std::endl;
    }
}
