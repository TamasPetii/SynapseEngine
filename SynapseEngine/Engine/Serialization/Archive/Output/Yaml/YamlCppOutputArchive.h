#pragma once
#include "Engine/SynApi.h"
#include "IYamlOutputArchive.h"
#include <yaml-cpp/yaml.h>
#include <vector>
#include <string>

namespace Syn
{
    class SYN_API YamlCppOutputArchive : public IYamlOutputArchive
    {
    public:
        static std::vector<std::string> GetSupportedExtensions() { return { ".yaml", ".yml" }; }

        explicit YamlCppOutputArchive(IOutputStream& stream);
        ~YamlCppOutputArchive() override = default;

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
        template<typename T>
        void WriteValue(const char* name, const T& value);

        YAML::Node _root;
        std::vector<YAML::Node> _stack;
    };

    template<typename T>
    void YamlCppOutputArchive::WriteValue(const char* name, const T& value)
    {
        YAML::Node parent = _stack.back();
        if (parent.IsSequence()) {
            parent.push_back(value);
        }
        else {
            parent[name] = value;
        }
    }
}