#include "DirectionLightViewModel.h"

namespace Syn {

    DirectionLightViewModel::DirectionLightViewModel(ISelectionApi* selectionApi, IDirectionLightApi* lightApi)
        : _selectionApi(selectionApi), _lightApi(lightApi)
    {}

    const DirectionLightState& DirectionLightViewModel::GetState() const {
        return _state;
    }

    void DirectionLightViewModel::SyncWithEngine() {
        if (!_selectionApi || !_lightApi) return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _lightApi->HasDirectionLight(activeEntity)) {
            _state.hasComponent = true;

            if (!_colorDrag.IsDragging())    _state.color = _lightApi->GetLightColor(activeEntity);
            if (!_strengthDrag.IsDragging()) _state.strength = _lightApi->GetLightStrength(activeEntity);

            _state.useShadow = _lightApi->GetLightUseShadow(activeEntity);

            if (_state.useShadow) {
                if (!_farPlaneDrag.IsDragging())      _state.shadowFarPlane = _lightApi->GetShadowFarPlane(activeEntity);
                if (!_cascadeSplitsDrag.IsDragging()) _state.cascadeSplits = _lightApi->GetCascadeSplits(activeEntity);
            }
        }
        else {
            _state.hasComponent = false;
        }
    }

    void DirectionLightViewModel::Dispatch(const DirectionLightIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, SetLightColorIntent>)
                HandleSetColor(arg);
            else if constexpr (std::is_same_v<T, SetLightStrengthIntent>)
                HandleSetStrength(arg);
            else if constexpr (std::is_same_v<T, SetLightUseShadowIntent>)
                HandleSetUseShadow(arg);
            else if constexpr (std::is_same_v<T, SetShadowFarPlaneIntent>)
                HandleSetFarPlane(arg);
            else if constexpr (std::is_same_v<T, SetCascadeSplitsIntent>)
                HandleSetCascadeSplits(arg);
            }, intent);
    }

    void DirectionLightViewModel::HandleSetColor(const SetLightColorIntent& intent) {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _colorDrag.Handle(
            intent.isDragging, intent.color, _state.color,

            [&](const glm::vec3& col) {
                _lightApi->SetLightColor(activeEntity, col);
            },

            [&](const glm::vec3& start, const glm::vec3& end) {
                return std::make_shared<ChangeLightColorCommand>(_lightApi, activeEntity, start, end);
            }
        );
    }

    void DirectionLightViewModel::HandleSetStrength(const SetLightStrengthIntent& intent) {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _strengthDrag.Handle(
            intent.isDragging, intent.strength, _state.strength,

            [&](const float& str) {
                _lightApi->SetLightStrength(activeEntity, str);
            },

            [&](const float& start, const float& end) {
                return std::make_shared<ChangeLightStrengthCommand>(_lightApi, activeEntity, start, end);
            }
        );
    }

    void DirectionLightViewModel::HandleSetUseShadow(const SetLightUseShadowIntent& intent) {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _lightApi->SetLightUseShadow(activeEntity, intent.useShadow);
    }

    void DirectionLightViewModel::HandleSetFarPlane(const SetShadowFarPlaneIntent& intent) {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _farPlaneDrag.Handle(
            intent.isDragging, intent.farPlane, _state.shadowFarPlane,

            [&](const float& farP) {
                _lightApi->SetShadowFarPlane(activeEntity, farP);
            },

            [&](const float& start, const float& end) {
                return std::make_shared<ChangeShadowFarPlaneCommand>(_lightApi, activeEntity, start, end);
            }
        );
    }

    void DirectionLightViewModel::HandleSetCascadeSplits(const SetCascadeSplitsIntent& intent) {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _cascadeSplitsDrag.Handle(
            intent.isDragging, intent.splits, _state.cascadeSplits,

            [&](const glm::vec4& spl) {
                _lightApi->SetCascadeSplits(activeEntity, spl);
            },

            [&](const glm::vec4& start, const glm::vec4& end) {
                return std::make_shared<ChangeCascadeSplitsCommand>(_lightApi, activeEntity, start, end);
            }
        );
    }
}