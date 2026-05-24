#include "NetworkInputStream.h"
#include <cstring>

namespace Syn
{
    NetworkInputStream::NetworkInputStream()
        : _readOffset(0), _isValid(true)
    {}

    void NetworkInputStream::AppendPacket(const void* data, size_t size)
    {
        if (size == 0) return;

        const uint8_t* byteData = static_cast<const uint8_t*>(data);

        _buffer.insert(_buffer.end(), byteData, byteData + size);
        _isValid = true;
    }

    void NetworkInputStream::ReadRaw(void* data, size_t size)
    {
        if (size == 0) return;

        if (_readOffset + size > _buffer.size())
        {
            _isValid = false;
            return;
        }

        std::memcpy(data, _buffer.data() + _readOffset, size);
        _readOffset += size;

        if (_readOffset == _buffer.size())
        {
            Reset();
        }
    }

    bool NetworkInputStream::IsValid() const
    {
        return _isValid;
    }

    void NetworkInputStream::Reset()
    {
        _buffer.clear();
        _readOffset = 0;
        _isValid = true;
    }
}