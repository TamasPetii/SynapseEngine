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
#include <string>
#include <unordered_map>
#include <mutex>

namespace Syn
{
    using TypeID = uint32_t;

    class SYN_API TypeManager {
    public:
        TypeManager(const TypeManager&) = delete;
        TypeManager& operator=(const TypeManager&) = delete;

        static TypeManager& Get();
        TypeID GetOrRegisterID(const std::string& typeName);
    private:
        TypeManager() = default;
        TypeID _counter = 0;
        std::mutex _mtx;
        std::unordered_map<std::string, TypeID> _registry;
    };
}