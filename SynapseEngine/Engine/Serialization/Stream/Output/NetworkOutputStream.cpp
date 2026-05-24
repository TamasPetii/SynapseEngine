#include "NetworkOutputStream.h"
#include <cstring>

namespace Syn
{
    NetworkOutputStream::NetworkOutputStream(SendPacketCallback sendCallback, size_t maxPacketSize)
        : _sendCallback(std::move(sendCallback)), _maxPacketSize(maxPacketSize)
    {
        _buffer.reserve(_maxPacketSize);
    }

    NetworkOutputStream::~NetworkOutputStream()
    {
        Flush();
    }

    void NetworkOutputStream::WriteRaw(const void* data, size_t size)
    {
        const uint8_t* byteData = static_cast<const uint8_t*>(data);
        size_t remainingSize = size;
        size_t offset = 0;

        while (remainingSize > 0)
        {
            size_t availableSpace = _maxPacketSize - _buffer.size();
            size_t writeSize = std::min(remainingSize, availableSpace);

            size_t currentBufferSize = _buffer.size();
            _buffer.resize(currentBufferSize + writeSize);
            std::memcpy(_buffer.data() + currentBufferSize, byteData + offset, writeSize);

            offset += writeSize;
            remainingSize -= writeSize;

            if (_buffer.size() >= _maxPacketSize)
            {
                Flush();
            }
        }
    }

    bool NetworkOutputStream::IsValid() const
    {
        return _sendCallback != nullptr;
    }

    void NetworkOutputStream::Flush()
    {
        if (!_buffer.empty() && _sendCallback)
        {
            _sendCallback(_buffer.data(), _buffer.size());
            _buffer.clear();
        }
    }
}