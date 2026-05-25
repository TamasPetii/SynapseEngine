#include "YamlCppInputArchive.h"

namespace Syn
{
    YamlCppInputArchive::YamlCppInputArchive(IInputStream& stream) : IYamlInputArchive(stream)
    {}

    void YamlCppInputArchive::Deserialize()
    {
        size_t size = _stream.GetSize();
        if (size > 0)
        {
            std::string yamlString(size, '\0');
            _stream.ReadRaw(yamlString.data(), size);

            _root = YAML::Load(yamlString);
            _stack.clear();
            _stack.push_back({ _root, 0 });
        }
    }

    void YamlCppInputArchive::EnterObject(const char* name)
    {
        auto& ctx = _stack.back();
        if (ctx.node.IsSequence()) {
            _stack.push_back({ ctx.node[ctx.sequenceIndex++], 0 });
        }
        else {
            _stack.push_back({ ctx.node[name], 0 });
        }
    }

    void YamlCppInputArchive::LeaveObject()
    {
        _stack.pop_back();
    }

    void YamlCppInputArchive::EnterArray(const char* name, uint32_t& size)
    {
        auto& ctx = _stack.back();
        YAML::Node targetNode;

        if (ctx.node.IsSequence()) {
            targetNode = ctx.node[ctx.sequenceIndex++];
        }
        else {
            targetNode = ctx.node[name];
        }

        size = static_cast<uint32_t>(targetNode.size());
        _stack.push_back({ targetNode, 0 });
    }

    void YamlCppInputArchive::LeaveArray()
    {
        _stack.pop_back();
    }

    void YamlCppInputArchive::PropertyBool(const char* name, bool& value) { 
        ReadValue(name, value);
    }

    void YamlCppInputArchive::PropertyUint8(const char* name, uint8_t& value)
    {
        uint32_t temp = 0;
        ReadValue(name, temp);
        value = static_cast<uint8_t>(temp);
    }

    void YamlCppInputArchive::PropertyInt32(const char* name, int32_t& value) { 
        ReadValue(name, value);
    }

    void YamlCppInputArchive::PropertyUint32(const char* name, uint32_t& value) { 
        ReadValue(name, value);
    }

    void YamlCppInputArchive::PropertyInt64(const char* name, int64_t& value) { 
        ReadValue(name, value);
    }

    void YamlCppInputArchive::PropertyUint64(const char* name, uint64_t& value) { 
        ReadValue(name, value);
    }

    void YamlCppInputArchive::PropertyFloat(const char* name, float& value) { 
        ReadValue(name, value);
    }

    void YamlCppInputArchive::PropertyDouble(const char* name, double& value) { 
        ReadValue(name, value);
    }

    void YamlCppInputArchive::PropertyString(const char* name, std::string& value) { 
        ReadValue(name, value);
    }

    void YamlCppInputArchive::PropertyBytes(const char* name, void* data, size_t size)
    {
        std::string hexText;
        ReadValue(name, hexText);

        if (hexText.empty() || !data || size == 0) return;

        uint8_t* outBytes = static_cast<uint8_t*>(data);
        size_t hexLen = hexText.length();

        for (size_t i = 0; i < size && (i * 2 + 1) < hexLen; ++i) {
            auto fromHex = [](char c) -> uint8_t {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return 0;
                };
            outBytes[i] = (fromHex(hexText[i * 2]) << 4) | fromHex(hexText[i * 2 + 1]);
        }
    }
}