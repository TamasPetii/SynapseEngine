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

#include "FileOutputStream.h"
#include <stdexcept>

namespace Syn
{
    FileOutputStream::FileOutputStream(const std::filesystem::path& path)
    {
        _stream.open(path, std::ios::binary | std::ios::out | std::ios::trunc);

        if (!_stream.is_open())
        {
            throw std::runtime_error("Failed to open FileOutputStream: " + path.string());
        }
    }

    FileOutputStream::~FileOutputStream()
    {
        if (_stream.is_open())
            _stream.close();
    }

    void FileOutputStream::WriteRaw(const void* data, size_t size)
    {
        if (size > 0 && _stream.good())
        {
            _stream.write(reinterpret_cast<const char*>(data), size);
        }
    }

    bool FileOutputStream::IsValid() const
    {
        return _stream.good();
    }
}