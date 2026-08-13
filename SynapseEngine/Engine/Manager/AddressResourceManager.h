// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include "BaseResourceManager.h"
#include "Engine/Utils/RenderBuffer.h"
#include "Engine/Utils/StaleBuffer.h"
#include <vector>
#include <mutex>
#include <memory>

namespace Syn 
{
    template <typename TResource, typename TAddressStruct>
    class AddressResourceManager : public BaseResourceManager<TResource> {
    public:
        AddressResourceManager(uint32_t framesInFlight, uint32_t initialCapacity, uint32_t upWindow, uint32_t downWindow);
        virtual ~AddressResourceManager() = default;

        void Update() override;
        void RecordSync(VkCommandBuffer cmd);

        VkBuffer GetAddressBufferHandle() const;
        VkDeviceAddress GetAddressBufferDeviceAddress() const;
    protected:
        virtual void OnEntryCreated(uint32_t index) override;
        void WriteAddress(uint32_t index, const TAddressStruct& addresses);
    protected:
        uint32_t _framesInFlight;
        std::mutex _staleMutex;
        std::mutex _writeMutex;
        RenderBuffer _addressBuffer;
        std::vector<StaleBuffer> _staleBuffers;
    };

    template <typename TResource, typename TAddressStruct>
    AddressResourceManager<TResource, TAddressStruct>::AddressResourceManager(uint32_t framesInFlight, uint32_t initialCapacity, uint32_t upWindow, uint32_t downWindow)
        : _framesInFlight(framesInFlight)
    {
        RenderBufferConfig config{};
        config.strategy = BufferStrategy::Hybrid;
        config.frames = 1;
        config.elementSize = static_cast<uint32_t>(sizeof(TAddressStruct));
        config.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        config.upWindow = upWindow;
        config.downWindow = downWindow;

        _addressBuffer.Initialize(config);
        _addressBuffer.UpdateCapacityAll(initialCapacity);
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
    VkBuffer AddressResourceManager<TResource, TAddressStruct>::GetAddressBufferHandle() const {
        return _addressBuffer.GetHandle(0);
    }

    template <typename TResource, typename TAddressStruct>
    VkDeviceAddress AddressResourceManager<TResource, TAddressStruct>::GetAddressBufferDeviceAddress() const {
        return _addressBuffer.GetAddress(0);
    }

    template <typename TResource, typename TAddressStruct>
    void AddressResourceManager<TResource, TAddressStruct>::RecordSync(VkCommandBuffer cmd) {
        _addressBuffer.RecordSync(cmd, 0);
    }

    template <typename TResource, typename TAddressStruct>
    void AddressResourceManager<TResource, TAddressStruct>::WriteAddress(uint32_t index, const TAddressStruct& addresses) {
        std::lock_guard<std::mutex> writeLock(_writeMutex);
        
        uint32_t requiredCapacity = index + 1;

        auto staleBuffers = _addressBuffer.UpdateCapacity(0, requiredCapacity);

        if (staleBuffers.HasAny()) {
            std::lock_guard<std::mutex> lock(_staleMutex);
            if (staleBuffers.mapped) _staleBuffers.push_back({ staleBuffers.mapped, _framesInFlight });
            if (staleBuffers.gpu)    _staleBuffers.push_back({ staleBuffers.gpu, _framesInFlight });
        }

        size_t offset = index * sizeof(TAddressStruct);
        _addressBuffer.Write(0, &addresses, sizeof(TAddressStruct), offset);
    }

    template <typename TResource, typename TAddressStruct>
    void AddressResourceManager<TResource, TAddressStruct>::OnEntryCreated(uint32_t index) {
        TAddressStruct emptyAddresses{};
        WriteAddress(index, emptyAddresses);
    }
}