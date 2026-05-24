#include "NlohmannJsonOutputArchive.h"

namespace Syn
{
    NlohmannJsonOutputArchive::NlohmannJsonOutputArchive(IOutputStream& stream) : IJsonOutputArchive(stream)
    {
        _root = nlohmann::json::object();
        _stack.push_back(&_root);
    }

    void NlohmannJsonOutputArchive::Serialize()
    {
        std::string jsonStr = ToString();
        _stream.WriteRaw(jsonStr.data(), jsonStr.size());
    }

    std::string NlohmannJsonOutputArchive::ToString() const
    {
        return _root.dump(4);
    }

    void NlohmannJsonOutputArchive::EnterObject(const char* name)
    {
        (*_stack.back())[name] = nlohmann::json::object();
        _stack.push_back(&((*_stack.back())[name]));
    }

    void NlohmannJsonOutputArchive::LeaveObject()
    {
        _stack.pop_back();
    }

    void NlohmannJsonOutputArchive::EnterArray(const char* name, size_t size)
    {
        (*_stack.back())[name] = nlohmann::json::array();
        _stack.push_back(&((*_stack.back())[name]));
    }

    void NlohmannJsonOutputArchive::LeaveArray()
    {
        _stack.pop_back();
    }

    void NlohmannJsonOutputArchive::PropertyBool(const char* name, bool value) { 
        (*_stack.back())[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyUint8(const char* name, uint8_t value) { 
        (*_stack.back())[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyInt32(const char* name, int32_t value) { 
        (*_stack.back())[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyUint32(const char* name, uint32_t value) { 
        (*_stack.back())[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyInt64(const char* name, int64_t value) { 
        (*_stack.back())[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyUint64(const char* name, uint64_t value) { 
        (*_stack.back())[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyFloat(const char* name, float value) { 
        (*_stack.back())[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyDouble(const char* name, double value) { 
        (*_stack.back())[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyString(const char* name, const std::string& value) { 
        (*_stack.back())[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyBytes(const char* name, const void* data, size_t size)
    {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        (*_stack.back())[name] = std::vector<uint8_t>(bytes, bytes + size);
    }
}