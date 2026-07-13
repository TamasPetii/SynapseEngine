#include "SpotLightViewModel.h"

namespace Syn
{
    SpotLightViewModel::SpotLightViewModel(ISelectionApi *selectionApi, ISpotLightApi *lightApi)
        : _selectionApi(selectionApi), _lightApi(lightApi) {}

    const SpotLightState &SpotLightViewModel::GetState() const { return _state; }

    void SpotLightViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_lightApi)
            return;
        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _lightApi->HasSpotLight(activeEntity))
        {
            _state.hasComponent = true;
            
            if (!_colorDrag.IsDragging())
                _state.color = _lightApi->GetLightColor(activeEntity);
            if (!_strengthDrag.IsDragging())
                _state.strength = _lightApi->GetLightStrength(activeEntity);
            if (!_rangeDrag.IsDragging())
                _state.range = _lightApi->GetLightRange(activeEntity);
            if (!_weakenDrag.IsDragging())
                _state.weakenDistance = _lightApi->GetLightWeakenDistance(activeEntity);
            if (!_innerAngleDrag.IsDragging())
                _state.innerAngle = _lightApi->GetLightInnerAngle(activeEntity);
            if (!_outerAngleDrag.IsDragging())
                _state.outerAngle = _lightApi->GetLightOuterAngle(activeEntity);

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

    void SpotLightViewModel::Dispatch(const SpotLightIntent &intent)
    {
        EntityID active = _selectionApi->GetSelectedEntity();
        if (active == NULL_ENTITY)
            return;

        std::visit([this, active](auto &&arg)
                   {
            using T = std::decay_t<decltype(arg)>;
            
            if constexpr (std::is_same_v<T, SetSpotLightColorIntent>) {
                _colorDrag.Handle(arg.isDragging, arg.color, _state.color, 
                    [&](const glm::vec3& v) { _lightApi->SetLightColor(active, v); },
                    [&](const glm::vec3& s, const glm::vec3& e) { return std::make_shared<ChangeSpotLightColorCommand>(_lightApi, active, s, e); });
            }
            else if constexpr (std::is_same_v<T, SetSpotLightStrengthIntent>) {
                _strengthDrag.Handle(arg.isDragging, arg.strength, _state.strength, 
                    [&](const float& v) { _lightApi->SetLightStrength(active, v); },
                    [&](const float& s, const float& e) { return std::make_shared<ChangeSpotLightStrengthCommand>(_lightApi, active, s, e); });
            }
            else if constexpr (std::is_same_v<T, SetSpotLightRangeIntent>) {
                _rangeDrag.Handle(arg.isDragging, arg.range, _state.range, 
                    [&](const float& v) { _lightApi->SetLightRange(active, v); },
                    [&](const float& s, const float& e) { return std::make_shared<ChangeSpotLightRangeCommand>(_lightApi, active, s, e); });
            }
            else if constexpr (std::is_same_v<T, SetSpotLightWeakenIntent>) {
                _weakenDrag.Handle(arg.isDragging, arg.distance, _state.weakenDistance, 
                    [&](const float& v) { _lightApi->SetLightWeakenDistance(active, v); },
                    [&](const float& s, const float& e) { return std::make_shared<ChangeSpotLightWeakenCommand>(_lightApi, active, s, e); });
            }
            else if constexpr (std::is_same_v<T, SetSpotLightInnerAngleIntent>) {
                _innerAngleDrag.Handle(arg.isDragging, arg.angle, _state.innerAngle, 
                    [&](const float& v) { _lightApi->SetLightInnerAngle(active, v); },
                    [&](const float& s, const float& e) { return std::make_shared<ChangeSpotLightInnerAngleCommand>(_lightApi, active, s, e); });
            }
            else if constexpr (std::is_same_v<T, SetSpotLightOuterAngleIntent>) {
                _outerAngleDrag.Handle(arg.isDragging, arg.angle, _state.outerAngle, 
                    [&](const float& v) { _lightApi->SetLightOuterAngle(active, v); },
                    [&](const float& s, const float& e) { return std::make_shared<ChangeSpotLightOuterAngleCommand>(_lightApi, active, s, e); });
            }
            else if constexpr (std::is_same_v<T, SetSpotLightUseShadowIntent>) {
                _state.useShadow = arg.useShadow;
                _lightApi->SetLightUseShadow(active, arg.useShadow);
            }
            else if constexpr (std::is_same_v<T, SetSpotLightShadowNearIntent>) {
                _nearPlaneDrag.Handle(arg.isDragging, arg.nearPlane, _state.shadowNearPlane, 
                    [&](const float& v) { _lightApi->SetShadowNearPlane(active, v); },
                    [&](const float& s, const float& e) { return std::make_shared<ChangeSpotLightShadowNearCommand>(_lightApi, active, s, e); });
            }
            else if constexpr (std::is_same_v<T, SetSpotLightShadowFarIntent>) {
                _farPlaneDrag.Handle(arg.isDragging, arg.farPlane, _state.shadowFarPlane, 
                    [&](const float& v) { _lightApi->SetShadowFarPlane(active, v); },
                    [&](const float& s, const float& e) { return std::make_shared<ChangeSpotLightShadowFarCommand>(_lightApi, active, s, e); });
            } }, intent);
    }
}