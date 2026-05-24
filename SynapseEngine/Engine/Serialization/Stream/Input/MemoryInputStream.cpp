#include "MemoryInputStream.h"
#include <cstring>

namespace Syn
{
    MemoryInputStream::MemoryInputStream(std::span<const uint8_t> data)
        : _data(data), _offset(0), _isValid(true)
    {}

    void MemoryInputStream::ReadRaw(void* data, size_t size)
    {
        if (size == 0) return;

        if (_offset + size > _data.size())
        {
            _isValid = false;
            return;
        }

        std::memcpy(data, _data.data() + _offset, size);
        _offset += size;
    }

    bool MemoryInputStream::IsValid() const
    {
        return _isValid;
    }
}