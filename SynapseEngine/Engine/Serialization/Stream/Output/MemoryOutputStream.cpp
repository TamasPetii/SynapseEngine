#include "MemoryOutputStream.h"
#include <cstring>

namespace Syn
{
    MemoryOutputStream::MemoryOutputStream(size_t reserveSize)
    {
        _buffer.reserve(reserveSize);
    }

    void MemoryOutputStream::WriteRaw(const void* data, size_t size)
    {
        if (size == 0) return;

        size_t oldSize = _buffer.size();
        _buffer.resize(oldSize + size);
        std::memcpy(_buffer.data() + oldSize, data, size);
    }

    bool MemoryOutputStream::IsValid() const
    {
        return true;
    }

    const std::vector<uint8_t>& MemoryOutputStream::GetBuffer() const
    {
        return _buffer;
    }

    std::vector<uint8_t> MemoryOutputStream::ConsumeBuffer()
    {
        return std::move(_buffer);
    }
}