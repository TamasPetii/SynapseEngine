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