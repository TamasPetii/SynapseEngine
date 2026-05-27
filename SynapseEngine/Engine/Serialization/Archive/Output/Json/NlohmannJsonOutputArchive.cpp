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
        auto& parent = *_stack.back();
        if (parent.is_array()) {
            parent.push_back(nlohmann::json::object());
            _stack.push_back(&parent.back());
        }
        else {
            parent[name] = nlohmann::json::object();
            _stack.push_back(&parent[name]);
        }
    }

    void NlohmannJsonOutputArchive::LeaveObject()
    {
        _stack.pop_back();
    }

    void NlohmannJsonOutputArchive::EnterArray(const char* name, uint32_t size)
    {
        auto& parent = *_stack.back();
        if (parent.is_array()) {
            parent.push_back(nlohmann::json::array());
            _stack.push_back(&parent.back());
        }
        else {
            parent[name] = nlohmann::json::array();
            _stack.push_back(&parent[name]);
        }
    }

    void NlohmannJsonOutputArchive::LeaveArray()
    {
        _stack.pop_back();
    }

    void NlohmannJsonOutputArchive::PropertyBool(const char* name, bool value) {
        auto& parent = *_stack.back();
        if (parent.is_array()) parent.push_back(value);
        else parent[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyUint8(const char* name, uint8_t value) {
        auto& parent = *_stack.back();
        if (parent.is_array()) parent.push_back(value);
        else parent[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyInt32(const char* name, int32_t value) {
        auto& parent = *_stack.back();
        if (parent.is_array()) parent.push_back(value);
        else parent[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyUint32(const char* name, uint32_t value) {
        auto& parent = *_stack.back();
        if (parent.is_array()) parent.push_back(value);
        else parent[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyInt64(const char* name, int64_t value) {
        auto& parent = *_stack.back();
        if (parent.is_array()) parent.push_back(value);
        else parent[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyUint64(const char* name, uint64_t value) {
        auto& parent = *_stack.back();
        if (parent.is_array()) parent.push_back(value);
        else parent[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyFloat(const char* name, float value) {
        auto& parent = *_stack.back();
        if (parent.is_array()) parent.push_back(value);
        else parent[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyDouble(const char* name, double value) {
        auto& parent = *_stack.back();
        if (parent.is_array()) parent.push_back(value);
        else parent[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyString(const char* name, const std::string& value) {
        auto& parent = *_stack.back();
        if (parent.is_array()) parent.push_back(value);
        else parent[name] = value;
    }

    void NlohmannJsonOutputArchive::PropertyBytes(const char* name, const void* data, size_t size)
    {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        auto vec = std::vector<uint8_t>(bytes, bytes + size);

        auto& parent = *_stack.back();
        if (parent.is_array()) parent.push_back(vec);
        else parent[name] = vec;
    }
}