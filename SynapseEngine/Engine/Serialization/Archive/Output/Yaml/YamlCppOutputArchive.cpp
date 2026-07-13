#include "YamlCppOutputArchive.h"

namespace Syn
{
    YamlCppOutputArchive::YamlCppOutputArchive(IOutputStream& stream) : IYamlOutputArchive(stream)
    {
        _root = YAML::Node(YAML::NodeType::Map);
        _stack.push_back(_root);
    }

    std::string YamlCppOutputArchive::ToString() const
    {
        YAML::Emitter emitter;
        emitter << _root;
        return std::string(emitter.c_str());
    }

    void YamlCppOutputArchive::Serialize()
    {
        std::string yamlStr = ToString();
        _stream.WriteRaw(yamlStr.data(), yamlStr.size());
    }

    void YamlCppOutputArchive::EnterObject(const char* name)
    {
        YAML::Node parent = _stack.back();
        YAML::Node child(YAML::NodeType::Map);

        if (parent.IsSequence()) {
            parent.push_back(child);
            _stack.push_back(parent[parent.size() - 1]);
        }
        else {
            parent[name] = child;
            _stack.push_back(parent[name]);
        }
    }

    void YamlCppOutputArchive::LeaveObject()
    {
        _stack.pop_back();
    }

    void YamlCppOutputArchive::EnterArray(const char* name, uint32_t size)
    {
        YAML::Node parent = _stack.back();
        YAML::Node child(YAML::NodeType::Sequence);

        if (parent.IsSequence()) {
            parent.push_back(child);
            _stack.push_back(parent[parent.size() - 1]);
        }
        else {
            parent[name] = child;
            _stack.push_back(parent[name]);
        }
    }

    void YamlCppOutputArchive::LeaveArray()
    {
        _stack.pop_back();
    }

    void YamlCppOutputArchive::PropertyBool(const char* name, bool value) { 
        WriteValue(name, value);
    }

    void YamlCppOutputArchive::PropertyUint8(const char* name, uint8_t value) { 
        WriteValue(name, static_cast<uint32_t>(value));
    }

    void YamlCppOutputArchive::PropertyUint16(const char* name, uint16_t value) {
        WriteValue(name, static_cast<uint32_t>(value));
    }

    void YamlCppOutputArchive::PropertyInt32(const char* name, int32_t value) { 
        WriteValue(name, value);
    }

    void YamlCppOutputArchive::PropertyUint32(const char* name, uint32_t value) { 
        WriteValue(name, value); 
    }

    void YamlCppOutputArchive::PropertyInt64(const char* name, int64_t value) { 
        WriteValue(name, value);
    }

    void YamlCppOutputArchive::PropertyUint64(const char* name, uint64_t value) { 
        WriteValue(name, value);
    }

    void YamlCppOutputArchive::PropertyFloat(const char* name, float value) { 
        WriteValue(name, value);
    }

    void YamlCppOutputArchive::PropertyDouble(const char* name, double value) { 
        WriteValue(name, value);
    }

    void YamlCppOutputArchive::PropertyString(const char* name, const std::string& value) { 
        WriteValue(name, value);
    }

    void YamlCppOutputArchive::PropertyBytes(const char* name, const void* data, size_t size)
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