#pragma once
#include "Engine/SynApi.h"
#include "IYamlInputArchive.h"
#include <yaml-cpp/yaml.h>
#include <vector>
#include <string>

namespace Syn
{
    class SYN_API YamlCppInputArchive : public IYamlInputArchive
    {
    public:
        static std::vector<std::string> GetSupportedExtensions() { return { ".yaml", ".yml" }; }

        explicit YamlCppInputArchive(IInputStream& stream);
        ~YamlCppInputArchive() override = default;

        void Deserialize() override;

        void EnterObject(const char* name) override;
        void LeaveObject() override;
        void EnterArray(const char* name, uint32_t& size) override;
        void LeaveArray() override;

        void PropertyBool(const char* name, bool& value) override;
        void PropertyUint8(const char* name, uint8_t& value) override;
        void PropertyInt32(const char* name, int32_t& value) override;
        void PropertyUint32(const char* name, uint32_t& value) override;
        void PropertyInt64(const char* name, int64_t& value) override;
        void PropertyUint64(const char* name, uint64_t& value) override;
        void PropertyFloat(const char* name, float& value) override;
        void PropertyDouble(const char* name, double& value) override;
        void PropertyString(const char* name, std::string& value) override;
        void PropertyBytes(const char* name, void* data, size_t size) override;
    private:
        struct ContextNode {
            YAML::Node node;
            size_t sequenceIndex = 0;
        };

        template<typename T>
        void ReadValue(const char* name, T& value);

        YAML::Node _root;
        std::vector<ContextNode> _stack;
    };

    template<typename T>
    void YamlCppInputArchive::ReadValue(const char* name, T& value)
    {
        auto& ctx = _stack.back();
        if (ctx.node.IsSequence()) {
            value = ctx.node[ctx.sequenceIndex++].as<T>();
        }
        else {
            value = ctx.node[name].as<T>();
        }
    }
}