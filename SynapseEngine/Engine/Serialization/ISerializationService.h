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
#include "Engine/Serialization/Archive/IArchiveRegistry.h"

namespace Syn
{
    template <typename Derived>
    class SYN_API ISerializationService {
    public:
        IArchiveRegistry& GetRegistry() {
            return static_cast<Derived*>(this)->GetRegistryImpl();
        }

        template<typename T>
        void Load(IInputStream& stream, const std::string& extension, T& outData) {
            static_cast<Derived*>(this)->template LoadImpl<T>(stream, extension, outData);
        }

        template<typename T>
        void Save(IOutputStream& stream, const std::string& extension, const T& data) {
            static_cast<Derived*>(this)->template SaveImpl<T>(stream, extension, data);
        }
    };
}