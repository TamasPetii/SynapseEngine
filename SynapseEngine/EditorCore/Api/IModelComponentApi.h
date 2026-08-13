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
#include "IApi.h"
#include "EditorCore/Types/EntityHandle.h"
#include <string>
#include <vector>
#include <cstdint>

namespace Syn {
    class IModelComponentApi : public IApi {
    public:
        virtual ~IModelComponentApi() = default;

        virtual bool HasModelComponent(EntityID entity) const = 0;

        virtual bool GetCastShadow(EntityID entity) const = 0;
        virtual bool GetReceiveShadow(EntityID entity) const = 0;
        virtual uint32_t GetModelIndex(EntityID entity) const = 0;

        virtual void SetCastShadow(EntityID entity, bool cast) = 0;
        virtual void SetReceiveShadow(EntityID entity, bool receive) = 0;
        virtual void SetModelIndex(EntityID entity, uint32_t index) = 0;

        virtual std::vector<std::pair<uint32_t, std::string>> GetAvailableModels() const = 0;
    };
}