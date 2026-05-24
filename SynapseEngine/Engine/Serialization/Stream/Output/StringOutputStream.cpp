#include "StringOutputStream.h"

namespace Syn
{
    void StringOutputStream::WriteRaw(const void* data, size_t size)
    {
        if (size > 0 && data != nullptr)
        {
            _buffer.append(reinterpret_cast<const char*>(data), size);
        }
    }

    bool StringOutputStream::IsValid() const
    {
        return true;
    }

    std::string StringOutputStream::ConsumeString()
    {
        return std::move(_buffer);
    }
}