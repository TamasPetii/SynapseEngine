#include "StringInputStream.h"
#include <cstring> // memcpy

namespace Syn
{
    StringInputStream::StringInputStream(std::string data)
        : _data(std::move(data))
    {}

    void StringInputStream::ReadRaw(void* data, size_t size)
    {
        if (size > 0 && IsValid())
        {
            size_t available = _data.size() - _position;
            size_t toRead = std::min(size, available);

            std::memcpy(data, _data.data() + _position, toRead);
            _position += toRead;
        }
    }

    bool StringInputStream::IsValid() const
    {
        return _position < _data.size();
    }
}