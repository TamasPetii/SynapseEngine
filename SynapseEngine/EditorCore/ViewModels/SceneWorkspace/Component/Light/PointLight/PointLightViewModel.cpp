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

#include "PointLightViewModel.h"

namespace Syn
{

    PointLightViewModel::PointLightViewModel(ISelectionApi *selectionApi, IPointLightApi *lightApi)
        : _selectionApi(selectionApi), _lightApi(lightApi)
    {
    }

    const PointLightState &PointLightViewModel::GetState() const
    {
        return _state;
    }

    void PointLightViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_lightApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _lightApi->HasPointLight(activeEntity))
        {
            _state.hasComponent = true;

            if (!_colorDrag.IsDragging())
                _state.color = _lightApi->GetLightColor(activeEntity);
            if (!_strengthDrag.IsDragging())
                _state.strength = _lightApi->GetLightStrength(activeEntity);
            if (!_radiusDrag.IsDragging())
                _state.radius = _lightApi->GetLightRadius(activeEntity);
            if (!_weakenDrag.IsDragging())
                _state.weakenDistance = _lightApi->GetLightWeakenDistance(activeEntity);

            _state.useShadow = _lightApi->GetLightUseShadow(activeEntity);

            if (_state.useShadow)
            {
                if (!_nearPlaneDrag.IsDragging())
                    _state.shadowNearPlane = _lightApi->GetShadowNearPlane(activeEntity);
                if (!_farPlaneDrag.IsDragging())
                    _state.shadowFarPlane = _lightApi->GetShadowFarPlane(activeEntity);
            }
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void PointLightViewModel::Dispatch(const PointLightIntent &intent)
    {
        std::visit([this](auto &&arg)
                   {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, SetPointLightColorIntent>)           HandleSetColor(arg);
            else if constexpr (std::is_same_v<T, SetPointLightStrengthIntent>)   HandleSetStrength(arg);
            else if constexpr (std::is_same_v<T, SetPointLightUseShadowIntent>)  HandleSetUseShadow(arg);
            else if constexpr (std::is_same_v<T, SetPointLightRadiusIntent>)     HandleSetRadius(arg);
            else if constexpr (std::is_same_v<T, SetPointLightWeakenIntent>)     HandleSetWeaken(arg);
            else if constexpr (std::is_same_v<T, SetPointLightShadowNearIntent>) HandleSetNearPlane(arg);
            else if constexpr (std::is_same_v<T, SetPointLightShadowFarIntent>)  HandleSetFarPlane(arg); }, intent);
    }

    void PointLightViewModel::HandleSetColor(const SetPointLightColorIntent &intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY)
            return;
        _colorDrag.Handle(intent.isDragging, intent.color, _state.color, [&](const glm::vec3 &v)
                          { _lightApi->SetLightColor(activeEntity, v); }, [&](const glm::vec3 &s, const glm::vec3 &e)
                          { return std::make_shared<ChangePointLightColorCommand>(_lightApi, activeEntity, s, e); });
    }

    void PointLightViewModel::HandleSetStrength(const SetPointLightStrengthIntent &intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY)
            return;
        _strengthDrag.Handle(intent.isDragging, intent.strength, _state.strength, [&](const float &v)
                             { _lightApi->SetLightStrength(activeEntity, v); }, [&](const float &s, const float &e)
                             { return std::make_shared<ChangePointLightStrengthCommand>(_lightApi, activeEntity, s, e); });
    }

    void PointLightViewModel::HandleSetRadius(const SetPointLightRadiusIntent &intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY)
            return;
        _radiusDrag.Handle(intent.isDragging, intent.radius, _state.radius, [&](const float &v)
                           { _lightApi->SetLightRadius(activeEntity, v); }, [&](const float &s, const float &e)
                           { return std::make_shared<ChangePointLightRadiusCommand>(_lightApi, activeEntity, s, e); });
    }

    void PointLightViewModel::HandleSetWeaken(const SetPointLightWeakenIntent &intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY)
            return;
        _weakenDrag.Handle(intent.isDragging, intent.distance, _state.weakenDistance, [&](const float &v)
                           { _lightApi->SetLightWeakenDistance(activeEntity, v); }, [&](const float &s, const float &e)
                           { return std::make_shared<ChangePointLightWeakenCommand>(_lightApi, activeEntity, s, e); });
    }

    void PointLightViewModel::HandleSetUseShadow(const SetPointLightUseShadowIntent &intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY)
            return;
        _state.useShadow = intent.useShadow;
        _lightApi->SetLightUseShadow(activeEntity, intent.useShadow);
    }

    void PointLightViewModel::HandleSetNearPlane(const SetPointLightShadowNearIntent &intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY)
            return;
        _nearPlaneDrag.Handle(intent.isDragging, intent.nearPlane, _state.shadowNearPlane, [&](const float &v)
                              { _lightApi->SetShadowNearPlane(activeEntity, v); }, [&](const float &s, const float &e)
                              { return std::make_shared<ChangePointLightShadowNearCommand>(_lightApi, activeEntity, s, e); });
    }

    void PointLightViewModel::HandleSetFarPlane(const SetPointLightShadowFarIntent &intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY)
            return;
        _farPlaneDrag.Handle(intent.isDragging, intent.farPlane, _state.shadowFarPlane, [&](const float &v)
                             { _lightApi->SetShadowFarPlane(activeEntity, v); }, [&](const float &s, const float &e)
                             { return std::make_shared<ChangePointLightShadowFarCommand>(_lightApi, activeEntity, s, e); });
    }
}