#include "PlusPlusTomlInputArchive.h"

namespace Syn
{
    PlusPlusTomlInputArchive::PlusPlusTomlInputArchive(IInputStream& stream) : ITomlInputArchive(stream)
    {}

    toml::node* PlusPlusTomlInputArchive::GetNextNode(const char* name)
    {
        auto& current = _stack.back();
        if (current.type == NodeType::Array) {
            auto* arr = static_cast<toml::array*>(current.nodePtr);
            return arr->get(current.arrayIterator++);
        }
        else {
            auto* tbl = static_cast<toml::table*>(current.nodePtr);
            return tbl->get(name);
        }
    }

    void PlusPlusTomlInputArchive::Deserialize()
    {
        size_t size = _stream.GetSize();
        if (size > 0)
        {
            std::string tomlString(size, '\0');
            _stream.ReadRaw(tomlString.data(), size);

            _root = toml::parse(tomlString);
            _stack.clear();
            _stack.push_back({ &_root, NodeType::Table, 0 });
        }
    }

    bool PlusPlusTomlInputArchive::HasProperty(const char* name)
    {
        if (_stack.empty()) return false;

        auto& current = _stack.back();
        if (current.type == NodeType::Table) {
            auto* tbl = static_cast<toml::table*>(current.nodePtr);
            return tbl->contains(name);
        }

        return false;
    }

    void PlusPlusTomlInputArchive::EnterObject(const char* name)
    {
        toml::node* next = GetNextNode(name);
        _stack.push_back({ next, NodeType::Table, 0 });
    }

    void PlusPlusTomlInputArchive::LeaveObject()
    {
        _stack.pop_back();
    }

    void PlusPlusTomlInputArchive::EnterArray(const char* name, uint32_t& size)
    {
        toml::node* next = GetNextNode(name);
        auto* arr = static_cast<toml::array*>(next);
        size = static_cast<uint32_t>(arr->size());
        _stack.push_back({ next, NodeType::Array, 0 });
    }

    void PlusPlusTomlInputArchive::LeaveArray()
    {
        _stack.pop_back();
    }

    void PlusPlusTomlInputArchive::PropertyBool(const char* name, bool& value) { 
        value = GetNextNode(name)->as_boolean()->get();
    }

    void PlusPlusTomlInputArchive::PropertyUint8(const char* name, uint8_t& value) { 
        value = static_cast<uint8_t>(GetNextNode(name)->as_integer()->get());
    }

    void PlusPlusTomlInputArchive::PropertyUint16(const char* name, uint16_t& value) {
        value = static_cast<uint16_t>(GetNextNode(name)->as_integer()->get());
    }

    void PlusPlusTomlInputArchive::PropertyInt32(const char* name, int32_t& value) { 
        value = static_cast<int32_t>(GetNextNode(name)->as_integer()->get());
    }

    void PlusPlusTomlInputArchive::PropertyUint32(const char* name, uint32_t& value) { 
        value = static_cast<uint32_t>(GetNextNode(name)->as_integer()->get());
    }

    void PlusPlusTomlInputArchive::PropertyInt64(const char* name, int64_t& value) { 
        value = GetNextNode(name)->as_integer()->get();
    }

    void PlusPlusTomlInputArchive::PropertyUint64(const char* name, uint64_t& value) { 
        value = static_cast<uint64_t>(GetNextNode(name)->as_integer()->get());
    }

    void PlusPlusTomlInputArchive::PropertyFloat(const char* name, float& value) { 
        value = static_cast<float>(GetNextNode(name)->as_floating_point()->get());
    }

    void PlusPlusTomlInputArchive::PropertyDouble(const char* name, double& value) { 
        value = GetNextNode(name)->as_floating_point()->get();
    }

    void PlusPlusTomlInputArchive::PropertyString(const char* name, std::string& value) { 
        value = GetNextNode(name)->as_string()->get();
    }

    void PlusPlusTomlInputArchive::PropertyBytes(const char* name, void* data, size_t size)
    {
        std::string hexText = GetNextNode(name)->as_string()->get();
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