#include "WindowedBuffer.h"

namespace Syn
{
    WindowedBuffer::WindowedBuffer(const Vk::BufferConfig& baseConfig, uint32_t elementSize, uint32_t upWindow, uint32_t downWindow)
        : _baseConfig(baseConfig), _elementSize(elementSize), _upWindow(upWindow), _downWindow(downWindow)
    {
    }

    std::pair<bool, std::shared_ptr<Vk::Buffer>> WindowedBuffer::UpdateCapacity(uint64_t requiredElements)
    {
        if (requiredElements == 0) requiredElements = 1;

        bool needsResize = false;
        uint64_t newCapacity = _capacity;

        if (requiredElements > _capacity)
        {
            newCapacity = ((requiredElements + _upWindow - 1) / _upWindow) * _upWindow;
            needsResize = true;
        }
        else if (_capacity >= requiredElements + _downWindow)
        {
            newCapacity = ((requiredElements + _upWindow - 1) / _upWindow) * _upWindow;
            needsResize = true;
        }

        if (needsResize || !_buffer)
        {
            _capacity = newCapacity;
            _baseConfig.size = _capacity * _elementSize;
            auto newBuffer = std::make_shared<Vk::Buffer>(_baseConfig);

            std::shared_ptr<Vk::Buffer> oldBuffer = _buffer;

            if (_buffer)
            {
                bool isMappable = (_baseConfig.allocationFlags & VMA_ALLOCATION_CREATE_MAPPED_BIT) != 0;

                if (isMappable)
                {
                    void* src = _buffer->Map();
                    void* dst = newBuffer->Map();

                    if (src && dst)
                    {
                        size_t copySize = std::min(_buffer->GetSize(), newBuffer->GetSize());
                        std::memcpy(dst, src, copySize);
                    }

                    _buffer->Unmap();
                    newBuffer->Unmap();
                }
            }

            _buffer = std::move(newBuffer);
            return std::make_pair(true, oldBuffer);
        }

        return std::make_pair(false, nullptr);
    }
}