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