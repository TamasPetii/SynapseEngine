#include "BinaryOutputArchive.h"

namespace Syn
{
    void BinaryOutputArchive::Serialize()
    {
		//No need to do anything here since we write directly to the stream in the property methods.
    }

    void BinaryOutputArchive::EnterArray(const char*, size_t size)
    {
        uint32_t s = static_cast<uint32_t>(size);
        _stream.WriteRaw(&s, sizeof(uint32_t));
    }

    void BinaryOutputArchive::PropertyBool(const char*, bool value)
    {
        _stream.WriteRaw(&value, sizeof(bool));
    }

    void BinaryOutputArchive::PropertyUint8(const char*, uint8_t value)
    {
        _stream.WriteRaw(&value, sizeof(uint8_t));
    }

    void BinaryOutputArchive::PropertyInt32(const char*, int32_t value)
    {
        _stream.WriteRaw(&value, sizeof(int32_t));
    }

    void BinaryOutputArchive::PropertyUint32(const char*, uint32_t value)
    {
        _stream.WriteRaw(&value, sizeof(uint32_t));
    }

    void BinaryOutputArchive::PropertyInt64(const char*, int64_t value)
    {
        _stream.WriteRaw(&value, sizeof(int64_t));
    }

    void BinaryOutputArchive::PropertyUint64(const char*, uint64_t value)
    {
        _stream.WriteRaw(&value, sizeof(uint64_t));
    }

    void BinaryOutputArchive::PropertyFloat(const char*, float value)
    {
        _stream.WriteRaw(&value, sizeof(float));
    }

    void BinaryOutputArchive::PropertyDouble(const char*, double value)
    {
        _stream.WriteRaw(&value, sizeof(double));
    }

    void BinaryOutputArchive::PropertyString(const char*, const std::string& value)
    {
        uint32_t size = static_cast<uint32_t>(value.size());
        _stream.WriteRaw(&size, sizeof(uint32_t));
        if (size > 0)
            _stream.WriteRaw(value.data(), size);
    }

    void BinaryOutputArchive::PropertyBytes(const char*, const void* data, size_t size)
    {
        uint32_t s = static_cast<uint32_t>(size);
        _stream.WriteRaw(&s, sizeof(uint32_t));

        if (size > 0 && data != nullptr) {
            _stream.WriteRaw(data, size);
        }
    }
}