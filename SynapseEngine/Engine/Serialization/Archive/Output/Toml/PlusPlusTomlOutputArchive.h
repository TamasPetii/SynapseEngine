#pragma once
#include "Engine/SynApi.h"
#include "ITomlOutputArchive.h"
#include <toml++/toml.h>
#include <vector>
#include <string>

namespace Syn
{
    class SYN_API PlusPlusTomlOutputArchive : public ITomlOutputArchive
    {
    public:
        static std::vector<std::string> GetSupportedExtensions() { return { ".toml" }; }

        explicit PlusPlusTomlOutputArchive(IOutputStream& stream);
        ~PlusPlusTomlOutputArchive() override = default;

        std::string ToString() const override;
        void Serialize() override;

        void EnterObject(const char* name) override;
        void LeaveObject() override;
        void EnterArray(const char* name, uint32_t size) override;
        void LeaveArray() override;

        void PropertyBool(const char* name, bool value) override;
        void PropertyUint8(const char* name, uint8_t value) override;
        void PropertyUint16(const char* name, uint16_t value) override;
        void PropertyInt32(const char* name, int32_t value) override;
        void PropertyUint32(const char* name, uint32_t value) override;
        void PropertyInt64(const char* name, int64_t value) override;
        void PropertyUint64(const char* name, uint64_t value) override;
        void PropertyFloat(const char* name, float value) override;
        void PropertyDouble(const char* name, double value) override;
        void PropertyString(const char* name, const std::string& value) override;
        void PropertyBytes(const char* name, const void* data, size_t size) override;

    private:
        enum class NodeType { Table, Array };

        struct StackNode {
            toml::node* nodePtr;
            NodeType type;
        };

        template<typename T>
        void WriteValue(const char* name, T&& value);

        toml::table _root;
        std::vector<StackNode> _stack;
    };

    template<typename T>
    void PlusPlusTomlOutputArchive::WriteValue(const char* name, T&& value)
    {
        auto& current = _stack.back();
        if (current.type == NodeType::Array) {
            auto* parentArray = static_cast<toml::array*>(current.nodePtr);
            parentArray->push_back(std::forward<T>(value));
        }
        else {
            auto* parentTable = static_cast<toml::table*>(current.nodePtr);
            parentTable->insert_or_assign(name, std::forward<T>(value));
        }
    }
}