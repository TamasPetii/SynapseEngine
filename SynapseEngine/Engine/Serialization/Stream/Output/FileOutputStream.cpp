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