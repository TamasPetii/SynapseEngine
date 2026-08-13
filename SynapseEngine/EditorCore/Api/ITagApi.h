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
#include <string>
#include "IApi.h"
#include "EditorCore/Types/EntityHandle.h"

namespace Syn {
    class ITagApi : public IApi {
    public:
        virtual ~ITagApi() = default;

        virtual std::string GetEntityName(EntityID entity) const = 0;
        virtual void SetEntityName(EntityID entity, const std::string& name) = 0;

        virtual std::string GetEntityTag(EntityID entity) const = 0;
        virtual void SetEntityTag(EntityID entity, const std::string& tag) = 0;

        virtual bool IsEntityEnabled(EntityID entity) const = 0;
        virtual void SetEntityEnabled(EntityID entity, bool enabled) = 0;
    };
}