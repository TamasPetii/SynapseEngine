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
#include "EditorCore/Api/IModelComponentApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class ModelComponentApiImpl : public IModelComponentApi {
    public:
        ModelComponentApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        bool HasModelComponent(EntityID entity) const override;

        bool GetCastShadow(EntityID entity) const override;
        bool GetReceiveShadow(EntityID entity) const override;
        uint32_t GetModelIndex(EntityID entity) const override;

        void SetCastShadow(EntityID entity, bool cast) override;
        void SetReceiveShadow(EntityID entity, bool receive) override;
        void SetModelIndex(EntityID entity, uint32_t index) override;

        std::vector<std::pair<uint32_t, std::string>> GetAvailableModels() const override;
    private:
        SceneManager* _sceneManager;
    };
}