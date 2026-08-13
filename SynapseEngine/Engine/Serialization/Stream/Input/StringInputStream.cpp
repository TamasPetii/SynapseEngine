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