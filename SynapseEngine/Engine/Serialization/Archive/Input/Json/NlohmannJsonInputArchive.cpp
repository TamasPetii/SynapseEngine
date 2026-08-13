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

#include "NlohmannJsonInputArchive.h"
#include <stdexcept>

namespace Syn
{
	NlohmannJsonInputArchive::NlohmannJsonInputArchive(IInputStream& stream) 
        : IJsonInputArchive(stream)
    {        
    }

    void NlohmannJsonInputArchive::Deserialize()
    {
        size_t size = _stream.GetSize();
        if (size > 0)
        {
            std::string jsonString(size, '\0');
            _stream.ReadRaw(jsonString.data(), size);

            _root = nlohmann::json::parse(jsonString);

            _stack.clear();
            _stack.push_back({ &_root, 0 });
        }
    }

    bool NlohmannJsonInputArchive::HasProperty(const char* name)
    {
        if (_stack.empty()) return false;

        auto& ctx = _stack.back();
        if (ctx.jsonNode->is_object()) {
            return ctx.jsonNode->contains(name);
        }
        return false;
    }

    void NlohmannJsonInputArchive::EnterObject(const char* name)
    {
        auto& ctx = _stack.back();
        if (ctx.jsonNode->is_array()) {
            _stack.push_back({ &((*ctx.jsonNode)[ctx.arrayIndex++]), 0 });
        }
        else {
            _stack.push_back({ &((*ctx.jsonNode).at(name)), 0 });
        }
    }

    void NlohmannJsonInputArchive::LeaveObject()
    {
        _stack.pop_back();
    }

    void NlohmannJsonInputArchive::EnterArray(const char* name, uint32_t& size)
    {
        auto& ctx = _stack.back();
        nlohmann::json* targetNode = nullptr;

        if (ctx.jsonNode->is_array()) {
            targetNode = &((*ctx.jsonNode)[ctx.arrayIndex++]);
        }
        else {
            targetNode = &((*ctx.jsonNode).at(name));
        }

        _stack.push_back({ targetNode, 0 });
        size = static_cast<uint32_t>(targetNode->size());
    }

    void NlohmannJsonInputArchive::LeaveArray()
    {
        _stack.pop_back();
    }

    void NlohmannJsonInputArchive::PropertyBool(const char* name, bool& value) {
        ReadValue(name, value);
    }

    void NlohmannJsonInputArchive::PropertyUint8(const char* name, uint8_t& value) {
        ReadValue(name, value);
    }

    void NlohmannJsonInputArchive::PropertyUint16(const char* name, uint16_t& value) {
        ReadValue(name, value);
    }

    void NlohmannJsonInputArchive::PropertyInt32(const char* name, int32_t& value) {
        ReadValue(name, value);
    }

    void NlohmannJsonInputArchive::PropertyUint32(const char* name, uint32_t& value) {
        ReadValue(name, value);
    }

    void NlohmannJsonInputArchive::PropertyInt64(const char* name, int64_t& value) {
        ReadValue(name, value);
    }

    void NlohmannJsonInputArchive::PropertyUint64(const char* name, uint64_t& value) {
        ReadValue(name, value);
    }

    void NlohmannJsonInputArchive::PropertyFloat(const char* name, float& value) {
        ReadValue(name, value);
    }

    void NlohmannJsonInputArchive::PropertyDouble(const char* name, double& value) {
        ReadValue(name, value);
    }

    void NlohmannJsonInputArchive::PropertyString(const char* name, std::string& value) {
        ReadValue(name, value);
    }

    void NlohmannJsonInputArchive::PropertyBytes(const char* name, void* data, size_t size)
    {
        auto& ctx = _stack.back();
        std::vector<uint8_t> vec;

        if (ctx.jsonNode->is_array()) {
            vec = (*ctx.jsonNode)[ctx.arrayIndex++].get<std::vector<uint8_t>>();
        }
        else {
            vec = ctx.jsonNode->at(name).get<std::vector<uint8_t>>();
        }

        size_t copySize = std::min(size, vec.size());
        if (data && copySize > 0) {
            std::memcpy(data, vec.data(), copySize);
        }
    }
}