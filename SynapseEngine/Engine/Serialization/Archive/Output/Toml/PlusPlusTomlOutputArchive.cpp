#include "PlusPlusTomlOutputArchive.h"
#include <sstream>

namespace Syn
{
    PlusPlusTomlOutputArchive::PlusPlusTomlOutputArchive(IOutputStream& stream) : ITomlOutputArchive(stream)
    {
        _stack.push_back({ &_root, NodeType::Table });
    }

    std::string PlusPlusTomlOutputArchive::ToString() const
    {
        std::stringstream ss;
        ss << _root;
        return ss.str();
    }

    void PlusPlusTomlOutputArchive::Serialize()
    {
        std::string tomlStr = ToString();
        _stream.WriteRaw(tomlStr.data(), tomlStr.size());
    }

    void PlusPlusTomlOutputArchive::EnterObject(const char* name)
    {
        auto& current = _stack.back();

        if (current.type == NodeType::Array) {
            auto* parentArray = static_cast<toml::array*>(current.nodePtr);
            parentArray->push_back(toml::table{});
            _stack.push_back({ &parentArray->back(), NodeType::Table });
        }
        else {
            auto* parentTable = static_cast<toml::table*>(current.nodePtr);
            parentTable->insert_or_assign(name, toml::table{});
            _stack.push_back({ parentTable->get(name), NodeType::Table });
        }
    }

    void PlusPlusTomlOutputArchive::LeaveObject()
    {
        _stack.pop_back();
    }

    void PlusPlusTomlOutputArchive::EnterArray(const char* name, uint32_t size)
    {
        auto& current = _stack.back();

        if (current.type == NodeType::Array) {
            auto* parentArray = static_cast<toml::array*>(current.nodePtr);
            parentArray->push_back(toml::array{});
            _stack.push_back({ &parentArray->back(), NodeType::Array });
        }
        else {
            auto* parentTable = static_cast<toml::table*>(current.nodePtr);
            parentTable->insert_or_assign(name, toml::array{});
            _stack.push_back({ parentTable->get(name), NodeType::Array });
        }
    }

    void PlusPlusTomlOutputArchive::LeaveArray()
    {
        _stack.pop_back();
    }

    

    void PlusPlusTomlOutputArchive::PropertyBool(const char* name, bool value) { 
        WriteValue(name, value);
    }

    void PlusPlusTomlOutputArchive::PropertyUint8(const char* name, uint8_t value) { 
        WriteValue(name, static_cast<int64_t>(value));
    }

    void PlusPlusTomlOutputArchive::PropertyInt32(const char* name, int32_t value) { 
        WriteValue(name, value);
    }

    void PlusPlusTomlOutputArchive::PropertyUint32(const char* name, uint32_t value) { 
        WriteValue(name, static_cast<int64_t>(value)); 
    }

    void PlusPlusTomlOutputArchive::PropertyInt64(const char* name, int64_t value) { 
        WriteValue(name, value);
    }

    void PlusPlusTomlOutputArchive::PropertyUint64(const char* name, uint64_t value) { 
        WriteValue(name, static_cast<int64_t>(value));
    }

    void PlusPlusTomlOutputArchive::PropertyFloat(const char* name, float value) { 
        WriteValue(name, static_cast<double>(value));
    }

    void PlusPlusTomlOutputArchive::PropertyDouble(const char* name, double value) { 
        WriteValue(name, value);
    }

    void PlusPlusTomlOutputArchive::PropertyString(const char* name, const std::string& value) { 
        WriteValue(name, value);
    }

    void PlusPlusTomlOutputArchive::PropertyBytes(const char* name, const void* data, size_t size)
    {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        std::string hexStr;
        hexStr.reserve(size * 2);
        const char* hexChars = "0123456789ABCDEF";
        for (size_t i = 0; i < size; ++i) {
            hexStr.push_back(hexChars[(bytes[i] >> 4) & 0x0F]);
            hexStr.push_back(hexChars[bytes[i] & 0x0F]);
        }
        WriteValue(name, hexStr);
    }
}