// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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
        bool HasProperty(const char* name) override;

        void EnterObject(const char* name) override;
        void LeaveObject() override;
        void EnterArray(const char* name, uint32_t& size) override;
        void LeaveArray() override;

        void PropertyBool(const char* name, bool& value) override;
        void PropertyUint8(const char* name, uint8_t& value) override;
        void PropertyUint16(const char* name, uint16_t& value) override;
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