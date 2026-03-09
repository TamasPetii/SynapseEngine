#pragma once
#include "Engine/SynApi.h"
#include "Engine/Vk/Buffer/Buffer.h"
#include <memory>
#include <algorithm>

namespace Syn
{
    class SYN_API WindowedBuffer
    {
    public:
        WindowedBuffer(const Vk::BufferConfig& baseConfig, uint32_t elementSize, uint32_t upWindow = 256, uint32_t downWindow = 512);

        bool UpdateCapacity(uint32_t requiredElements);

        Vk::Buffer* GetBuffer() const { return _buffer.get(); }
        uint32_t GetCapacity() const { return _capacity; }
        uint32_t GetElementSize() const { return _elementSize; }
    private:
        Vk::BufferConfig _baseConfig;
        uint32_t _elementSize;
        uint32_t _upWindow;
        uint32_t _downWindow;
        uint32_t _capacity = 0;
        std::shared_ptr<Vk::Buffer> _buffer;
    };
}