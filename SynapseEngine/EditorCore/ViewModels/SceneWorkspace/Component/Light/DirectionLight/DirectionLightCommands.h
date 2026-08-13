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
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/IDirectionLightApi.h"
#include <glm/glm.hpp>

namespace Syn
{
    class ChangeLightColorCommand : public ICommand {
    public:
        ChangeLightColorCommand(IDirectionLightApi* api, EntityID entity, const glm::vec3& oldColor, const glm::vec3& newColor)
            : _api(api), _entity(entity), _oldColor(oldColor), _newColor(newColor) {}

        void Execute() override { _api->SetLightColor(_entity, _newColor); }
        void Undo() override { _api->SetLightColor(_entity, _oldColor); }

    private:
        IDirectionLightApi* _api;
        EntityID _entity;
        glm::vec3 _oldColor;
        glm::vec3 _newColor;
    };

    class ChangeLightStrengthCommand : public ICommand {
    public:
        ChangeLightStrengthCommand(IDirectionLightApi* api, EntityID entity, float oldStrength, float newStrength)
            : _api(api), _entity(entity), _oldStrength(oldStrength), _newStrength(newStrength) {}

        void Execute() override { _api->SetLightStrength(_entity, _newStrength); }
        void Undo() override { _api->SetLightStrength(_entity, _oldStrength); }

    private:
        IDirectionLightApi* _api;
        EntityID _entity;
        float _oldStrength;
        float _newStrength;
    };

    class ChangeLightUseShadowCommand : public ICommand {
    public:
        ChangeLightUseShadowCommand(IDirectionLightApi* api, EntityID entity, bool oldUseShadow, bool newUseShadow)
            : _api(api), _entity(entity), _oldUseShadow(oldUseShadow), _newUseShadow(newUseShadow) {}

        void Execute() override { _api->SetLightUseShadow(_entity, _newUseShadow); }
        void Undo() override { _api->SetLightUseShadow(_entity, _oldUseShadow); }

    private:
        IDirectionLightApi* _api;
        EntityID _entity;
        bool _oldUseShadow;
        bool _newUseShadow;
    };

    class ChangeShadowFarPlaneCommand : public ICommand {
    public:
        ChangeShadowFarPlaneCommand(IDirectionLightApi* api, EntityID entity, float oldFarPlane, float newFarPlane)
            : _api(api), _entity(entity), _oldFarPlane(oldFarPlane), _newFarPlane(newFarPlane) {}

        void Execute() override { _api->SetShadowFarPlane(_entity, _newFarPlane); }
        void Undo() override { _api->SetShadowFarPlane(_entity, _oldFarPlane); }

    private:
        IDirectionLightApi* _api;
        EntityID _entity;
        float _oldFarPlane;
        float _newFarPlane;
    };

    class ChangeCascadeSplitsCommand : public ICommand {
    public:
        ChangeCascadeSplitsCommand(IDirectionLightApi* api, EntityID entity, const glm::vec4& oldSplits, const glm::vec4& newSplits)
            : _api(api), _entity(entity), _oldSplits(oldSplits), _newSplits(newSplits) {}

        void Execute() override { _api->SetCascadeSplits(_entity, _newSplits); }
        void Undo() override { _api->SetCascadeSplits(_entity, _oldSplits); }

    private:
        IDirectionLightApi* _api;
        EntityID _entity;
        glm::vec4 _oldSplits;
        glm::vec4 _newSplits;
    };
}