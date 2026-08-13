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
#include "IXmlInputArchive.h"
#include <tinyxml2.h>
#include <vector>
#include <string>

namespace Syn
{
    class SYN_API TinyXmlInputArchive : public IXmlInputArchive
    {
    public:
        static std::vector<std::string> GetSupportedExtensions() { return { ".xml" }; }

        explicit TinyXmlInputArchive(IInputStream& stream);
        ~TinyXmlInputArchive() override = default;

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
            tinyxml2::XMLElement* node;
            bool isArray;
            tinyxml2::XMLElement* arrayIterator;
        };

        tinyxml2::XMLElement* GetNextElement(const char* name);
        tinyxml2::XMLDocument _doc;
        std::vector<ContextNode> _stack;
    };
}