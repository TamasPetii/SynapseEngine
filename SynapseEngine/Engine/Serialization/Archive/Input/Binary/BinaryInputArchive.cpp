#include "BinaryInputArchive.h"

namespace Syn
{
    void BinaryInputArchive::EnterArray(const char*, uint32_t& size)
    {
        uint32_t s;
        _stream.ReadRaw(&s, sizeof(uint32_t));
        size = static_cast<uint32_t>(s);
    }

    void BinaryInputArchive::PropertyBool(const char*, bool& value) { 
        _stream.ReadRaw(&value, sizeof(bool));
    }

    void BinaryInputArchive::PropertyUint8(const char*, uint8_t& value) { 
        _stream.ReadRaw(&value, sizeof(uint8_t)); 
    }

    void BinaryInputArchive::PropertyUint16(const char*, uint16_t& value) {
        _stream.ReadRaw(&value, sizeof(uint16_t));
    }

    void BinaryInputArchive::PropertyInt32(const char*, int32_t& value) { 
        _stream.ReadRaw(&value, sizeof(int32_t)); 
    }

    void BinaryInputArchive::PropertyUint32(const char*, uint32_t& value) { 
        _stream.ReadRaw(&value, sizeof(uint32_t)); 
    }

    void BinaryInputArchive::PropertyInt64(const char*, int64_t& value) { 
        _stream.ReadRaw(&value, sizeof(int64_t)); 
    }

    void BinaryInputArchive::PropertyUint64(const char*, uint64_t& value) { 
        _stream.ReadRaw(&value, sizeof(uint64_t)); 
    }

    void BinaryInputArchive::PropertyFloat(const char*, float& value) { 
        _stream.ReadRaw(&value, sizeof(float)); 
    }

    void BinaryInputArchive::PropertyDouble(const char*, double& value) { 
        _stream.ReadRaw(&value, sizeof(double)); 
    }

    void BinaryInputArchive::PropertyString(const char*, std::string& value)
    {
        uint32_t size;
        _stream.ReadRaw(&size, sizeof(uint32_t));
        value.resize(size);
        if (size > 0)
            _stream.ReadRaw(value.data(), size);
    }

    void BinaryInputArchive::PropertyBytes(const char*, void* data, size_t size)
    {
        uint32_t s;
        _stream.ReadRaw(&s, sizeof(uint32_t));
        if (size > 0 && data != nullptr) {
            _stream.ReadRaw(data, size);
        }
    }
}