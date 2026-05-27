#pragma once
#include "BaseResourceManager.h"
#include "Engine/Utils/WindowedBuffer.h"
#include <vector>
#include <mutex>
#include <memory>

namespace Syn 
{
    struct SYN_API StaleBuffer {
        std::shared_ptr<Vk::Buffer> buffer;
        uint32_t framesToLive;
    };

    template <typename TResource, typename TAddressStruct>
    class SYN_API AddressResourceManager : public BaseResourceManager<TResource> {
    public:
        AddressResourceManager(uint32_t framesInFlight, uint32_t initialCapacity, uint32_t upWindow, uint32_t downWindow);
        virtual ~AddressResourceManager() = default;

        void Update() override;
        Vk::Buffer* GetAddressBuffer() const;
    protected:
        void WriteAddress(uint32_t index, const TAddressStruct& addresses);
    protected:
        std::unique_ptr<WindowedBuffer> _addressBuffer;
        std::vector<StaleBuffer> _staleBuffers;
        std::mutex _staleMutex;
        uint32_t _framesInFlight;
    };

    template <typename TResource, typename TAddressStruct>
    AddressResourceManager<TResource, TAddressStruct>::AddressResourceManager(uint32_t framesInFlight, uint32_t initialCapacity, uint32_t upWindow, uint32_t downWindow)
        : _framesInFlight(framesInFlight)
    {
        Vk::BufferConfig config{};
        config.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        config.memoryUsage = VMA_MEMORY_USAGE_AUTO;
        config.allocationFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        config.useDeviceAddress = true;

        _addressBuffer = std::make_unique<WindowedBuffer>(config, sizeof(TAddressStruct), upWindow, downWindow);
        _addressBuffer->UpdateCapacity(initialCapacity);
    }

    template <typename TResource, typename TAddressStruct>
    void AddressResourceManager<TResource, TAddressStruct>::Update() {
        BaseResourceManager<TResource>::Update();

        std::lock_guard<std::mutex> lock(_staleMutex);
        for (auto it = _staleBuffers.begin(); it != _staleBuffers.end();) {
            if (it->framesToLive > 0) {
                it->framesToLive--;
                ++it;
            }
            else {
				std::printf("Destroying stale buffer with address: 0x%llx\n", it->buffer->GetDeviceAddress());
                it = _staleBuffers.erase(it);
            }
        }
    }

    template <typename TResource, typename TAddressStruct>
    Vk::Buffer* AddressResourceManager<TResource, TAddressStruct>::GetAddressBuffer() const {
        return _addressBuffer->GetBuffer();
    }

    template <typename TResource, typename TAddressStruct>
    void AddressResourceManager<TResource, TAddressStruct>::WriteAddress(uint32_t index, const TAddressStruct& addresses) {
        uint32_t requiredCapacity = index + 1;

		auto [resized, oldBuffer] = _addressBuffer->UpdateCapacity(requiredCapacity);

        if (resized) {
            std::lock_guard<std::mutex> lock(_staleMutex);
            _staleBuffers.push_back({ oldBuffer, _framesInFlight });
        }

        size_t offset = index * sizeof(TAddressStruct);
        _addressBuffer->GetBuffer()->Write(&addresses, sizeof(TAddressStruct), offset);
    }
}