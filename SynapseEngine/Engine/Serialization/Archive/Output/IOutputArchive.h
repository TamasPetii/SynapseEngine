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
#include "Engine/Serialization/Schema/Schema.h"
#include "Engine/Serialization/Archive/IArchive.h"
#include "Engine/Serialization/Stream/Output/IOutputStream.h"
#include <string>
#include <cstdint>
#include <type_traits>

namespace Syn
{
    class SYN_API IOutputArchive : public IArchive {
    public:
        explicit IOutputArchive(IOutputStream& stream) : _stream(stream) {}
        virtual ~IOutputArchive() = default;

        virtual bool IsBinary() const { return false; }

        virtual void Serialize() = 0;
        virtual void EnterObject(const char* name) = 0;
        virtual void LeaveObject() = 0;
        virtual void EnterArray(const char* name, uint32_t size) = 0;
        virtual void LeaveArray() = 0;

        virtual void PropertyBool(const char* name, bool value) = 0;
        virtual void PropertyUint8(const char* name, uint8_t value) = 0;
        virtual void PropertyUint16(const char* name, uint16_t value) = 0;
        virtual void PropertyInt32(const char* name, int32_t value) = 0;
        virtual void PropertyUint32(const char* name, uint32_t value) = 0;
        virtual void PropertyInt64(const char* name, int64_t value) = 0;
        virtual void PropertyUint64(const char* name, uint64_t value) = 0;
        virtual void PropertyFloat(const char* name, float value) = 0;
        virtual void PropertyDouble(const char* name, double value) = 0;
        virtual void PropertyString(const char* name, const std::string& value) = 0;
        virtual void PropertyBytes(const char* name, const void* data, size_t size) = 0;

        template <typename T>
        void Property(const char* name, const T& value);
    protected:
        IOutputStream& _stream;
    };

    template <typename T>
    void IOutputArchive::Property(const char* name, const T& value) {
        if constexpr (std::is_same_v<T, bool>) PropertyBool(name, value);
        else if constexpr (std::is_same_v<T, uint8_t>) PropertyUint8(name, value);
        else if constexpr (std::is_same_v<T, uint16_t>) PropertyUint16(name, value);
        else if constexpr (std::is_same_v<T, int32_t>) PropertyInt32(name, value);
        else if constexpr (std::is_same_v<T, uint32_t>) PropertyUint32(name, value);
        else if constexpr (std::is_same_v<T, int64_t>) PropertyInt64(name, value);
        else if constexpr (std::is_same_v<T, uint64_t>) PropertyUint64(name, value);
        else if constexpr (std::is_same_v<T, float>) PropertyFloat(name, value);
        else if constexpr (std::is_same_v<T, double>) PropertyDouble(name, value);
        else if constexpr (std::is_convertible_v<T, std::string>) PropertyString(name, static_cast<std::string>(value));
        else if constexpr (std::is_enum_v<T>) {
            Property(name, static_cast<std::underlying_type_t<T>>(value));
        }
        else if constexpr (has_schema<T, std::remove_reference_t<decltype(*this)>>) {
            Schema<std::remove_cvref_t<T>>::Invoke(*this, name, value);
        }
        else static_assert(has_schema<T>, "Serialization Error: Type has no defined Schema specialization!");
    }
}