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
#include "EditorCore/Api/ITagApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class TagApiImpl : public ITagApi {
    public:
        TagApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        std::string GetEntityName(EntityID entity) const override;
        void SetEntityName(EntityID entity, const std::string& name) override;
        std::string GetEntityTag(EntityID entity) const override;
        void SetEntityTag(EntityID entity, const std::string& tag) override;
        bool IsEntityEnabled(EntityID entity) const override;
        void SetEntityEnabled(EntityID entity, bool enabled) override;
    private:
        SceneManager* _sceneManager;
    };
}