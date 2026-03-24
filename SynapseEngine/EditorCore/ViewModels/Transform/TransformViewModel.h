#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Interaction/DragInteraction.h"
#include "TransformState.h"
#include "TransformIntent.h"
#include "TransformCommands.h"
#include "EditorCore/API/ISelectionAPI.h"
#include "EditorCore/API/ITransformAPI.h"

namespace Syn {
    class TransformViewModel : public IViewModel<TransformState, TransformIntent> {
    public:
        TransformViewModel(ISelectionAPI* selectionApi, ITransformAPI* transformApi) : _selectionApi(selectionApi), _transformApi(transformApi) {}

        const TransformState& GetState() const override { return _state; }

        void SyncWithEngine() override {
            if (!_selectionApi || !_transformApi) return;

            EntityID activeEntity = _selectionApi->GetSelectedEntity();

            if (activeEntity != NULL_ENTITY) {
                _state.hasSelection = true;
                _state.activeEntityId = activeEntity;

                if (!_positionDrag.IsDragging()) _state.position = _transformApi->GetEntityPosition(activeEntity);
                if (!_rotationDrag.IsDragging()) _state.rotation = _transformApi->GetEntityRotation(activeEntity);
                if (!_scaleDrag.IsDragging())    _state.scale = _transformApi->GetEntityScale(activeEntity);
            }
            else {
                _state.hasSelection = false;
            }
        }

        void Dispatch(const TransformIntent& intent) override {
            std::visit([this](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, SetPositionIntent>)
                    HandleSetPosition(arg);
                else if constexpr (std::is_same_v<T, SetRotationIntent>)
                    HandleSetRotation(arg);
                else if constexpr (std::is_same_v<T, SetScaleIntent>) 
                    HandleSetScale(arg);
                }, intent);
        }
    private:
        void HandleSetPosition(const SetPositionIntent& intent) {
            if (!_state.hasSelection) return;

            _positionDrag.Handle(
                intent.isDragging, intent.newPosition, _state.position,

                [&](const glm::vec3& pos) {
                    _transformApi->SetEntityPosition(_state.activeEntityId, pos);
                },

                [&](const glm::vec3& start, const glm::vec3& end) {
                    return std::make_shared<ChangePositionCommand>(_transformApi, _state.activeEntityId, start, end);
                }
            );
        }

        void HandleSetRotation(const SetRotationIntent& intent) {
            if (!_state.hasSelection) return;

            _rotationDrag.Handle(
                intent.isDragging, intent.newRotation, _state.rotation,

                [&](const glm::vec3& rot) { 
                    _transformApi->SetEntityRotation(_state.activeEntityId, rot);
                },

                [&](const glm::vec3& start, const glm::vec3& end) {
                    return std::make_shared<ChangeRotationCommand>(_transformApi, _state.activeEntityId, start, end);
                }
            );
        }

        void HandleSetScale(const SetScaleIntent& intent) {
            if (!_state.hasSelection) return;

            _scaleDrag.Handle(
                intent.isDragging, intent.newScale, _state.scale,

                [&](const glm::vec3& scl) { 
                    _transformApi->SetEntityScale(_state.activeEntityId, scl);
                },

                [&](const glm::vec3& start, const glm::vec3& end) {
                    return std::make_shared<ChangeScaleCommand>(_transformApi, _state.activeEntityId, start, end);
                }
            );
        }

    private:
        ISelectionAPI* _selectionApi = nullptr;
        ITransformAPI* _transformApi = nullptr;
        TransformState _state;

        DragInteraction<glm::vec3> _positionDrag;
        DragInteraction<glm::vec3> _rotationDrag;
        DragInteraction<glm::vec3> _scaleDrag;
    };
}