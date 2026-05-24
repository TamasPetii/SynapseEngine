#include "FileInputStream.h"
#include <stdexcept>

namespace Syn
{
    FileInputStream::FileInputStream(const std::filesystem::path& path)
    {
        _stream.open(path, std::ios::binary | std::ios::in);
        if (!_stream.is_open())
        {
            throw std::runtime_error("Failed to open FileInputStream: " + path.string());
        }
    }

    FileInputStream::~FileInputStream()
    {
        if (_stream.is_open())
            _stream.close();
    }

    size_t FileInputStream::GetSize()
    {
        if (!_stream.is_open()) 
            return 0;

        auto currentPos = _stream.tellg();
        _stream.seekg(0, std::ios::end);
        size_t size = _stream.tellg();
        _stream.seekg(currentPos, std::ios::beg);

        return size;
    }

    void FileInputStream::ReadRaw(void* data, size_t size)
    {
        if (size > 0 && _stream.good())
        {
            _stream.read(reinterpret_cast<char*>(data), size);
        }
    }

    bool FileInputStream::IsValid() const
    {
        return _stream.good();
    }
}