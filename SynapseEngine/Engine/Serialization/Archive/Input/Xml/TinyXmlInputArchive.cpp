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

#include "TinyXmlInputArchive.h"
#include <stdexcept>
#include <charconv>

namespace Syn
{
    TinyXmlInputArchive::TinyXmlInputArchive(IInputStream& stream) : IXmlInputArchive(stream)
    {}

    void TinyXmlInputArchive::Deserialize()
    {
        size_t size = _stream.GetSize();
        if (size > 0)
        {
            std::string xmlString(size, '\0');
            _stream.ReadRaw(xmlString.data(), size);

            _doc.Parse(xmlString.c_str(), size);

            _stack.clear();
            ContextNode dummyRoot = { (tinyxml2::XMLElement*)&_doc, false, nullptr };
            _stack.push_back(dummyRoot);
        }
    }

    bool TinyXmlInputArchive::HasProperty(const char* name)
    {
        if (_stack.empty()) return false;

        auto& ctx = _stack.back();
        if (ctx.isArray) return false;

        return ctx.node->FirstChildElement(name) != nullptr;
    }

    tinyxml2::XMLElement* TinyXmlInputArchive::GetNextElement(const char* name)
    {
        auto& ctx = _stack.back();
        if (ctx.isArray) {
            tinyxml2::XMLElement* current = ctx.arrayIterator;
            if (current) {
                ctx.arrayIterator = current->NextSiblingElement("item");
            }
            return current;
        }
        else {
            return ctx.node->FirstChildElement(name);
        }
    }

    void TinyXmlInputArchive::EnterObject(const char* name)
    {
        tinyxml2::XMLElement* el = GetNextElement(name);
        _stack.push_back({ el, false, nullptr });
    }

    void TinyXmlInputArchive::LeaveObject()
    {
        _stack.pop_back();
    }

    void TinyXmlInputArchive::EnterArray(const char* name, uint32_t& size)
    {
        tinyxml2::XMLElement* el = GetNextElement(name);

        unsigned int parsedSize = 0;
        if (el->QueryUnsignedAttribute("size", &parsedSize) != tinyxml2::XML_SUCCESS) {
            parsedSize = 0;
            for (auto child = el->FirstChildElement("item"); child != nullptr; child = child->NextSiblingElement("item")) {
                parsedSize++;
            }
        }

        size = static_cast<uint32_t>(parsedSize);
        _stack.push_back({ el, true, el->FirstChildElement("item") });
    }

    void TinyXmlInputArchive::LeaveArray()
    {
        _stack.pop_back();
    }

    void TinyXmlInputArchive::PropertyBool(const char* name, bool& value) { 
        value = GetNextElement(name)->BoolText();
    }

    void TinyXmlInputArchive::PropertyUint8(const char* name, uint8_t& value) { 
        value = static_cast<uint8_t>(GetNextElement(name)->UnsignedText()); 
    }

    void TinyXmlInputArchive::PropertyUint16(const char* name, uint16_t& value) {
        value = static_cast<uint16_t>(GetNextElement(name)->UnsignedText());
    }

    void TinyXmlInputArchive::PropertyInt32(const char* name, int32_t& value) { 
        value = GetNextElement(name)->IntText(); 
    }

    void TinyXmlInputArchive::PropertyUint32(const char* name, uint32_t& value) { 
        value = GetNextElement(name)->UnsignedText(); 
    }

    void TinyXmlInputArchive::PropertyInt64(const char* name, int64_t& value) { 
        value = GetNextElement(name)->Int64Text(); 
    }

    void TinyXmlInputArchive::PropertyUint64(const char* name, uint64_t& value) { 
        value = GetNextElement(name)->Unsigned64Text(); 
    }

    void TinyXmlInputArchive::PropertyFloat(const char* name, float& value) { 
        value = GetNextElement(name)->FloatText(); 
    }

    void TinyXmlInputArchive::PropertyDouble(const char* name, double& value) { 
        value = GetNextElement(name)->DoubleText(); 
    }

    void TinyXmlInputArchive::PropertyString(const char* name, std::string& value)
    {
        const char* text = GetNextElement(name)->GetText();
        value = text ? text : "";
    }

    void TinyXmlInputArchive::PropertyBytes(const char* name, void* data, size_t size)
    {
        const char* hexText = GetNextElement(name)->GetText();
        if (!hexText || !data || size == 0) return;

        uint8_t* outBytes = static_cast<uint8_t*>(data);
        size_t hexLen = std::strlen(hexText);

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