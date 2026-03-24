#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "ViewportState.h"
#include "ViewportIntent.h"
#include "EditorCore/API/IRenderAPI.h"
#include "EditorCore/API/ISelectionAPI.h"
#include "EditorCore/API/ITransformAPI.h"
#include <glm/gtc/type_ptr.hpp>

namespace Syn {
    class ViewportViewModel : public IViewModel<ViewportState, ViewportIntent> {
    public:
        ViewportViewModel(IRenderAPI* renderApi, ISelectionAPI* selectionApi, ITransformAPI* transformApi)
            : _renderApi(renderApi), _selectionApi(selectionApi), _transformApi(transformApi) {}

        const ViewportState& GetState() const override { return _state; }

        void SyncWithEngine() override {
            if (!_renderApi) return;

            _state.textureId = _renderApi->GetViewportTexture(_state.currentGroup, _state.currentTarget, _state.currentView);

            _state.cameraView = _renderApi->GetEditorCameraView();
            _state.cameraProj = _renderApi->GetEditorCameraProjection();

            _state.activeEntity = _selectionApi->GetSelectedEntity();

            if (_state.activeEntity != NULL_ENTITY) {
                _state.entityWorldTransform = _transformApi->GetEntityWorldMatrix(_state.activeEntity);

                EntityID parentId = _transformApi->GetEntityParent(_state.activeEntity);
                if (parentId != NULL_ENTITY) {
                    _state.hasParent = true;
                    _state.parentWorldTransform = _transformApi->GetEntityWorldMatrix(parentId);
                }
                else {
                    _state.hasParent = false;
                    _state.parentWorldTransform = glm::mat4(1.0f);
                }
            }
        }

        void Dispatch(const ViewportIntent& intent) override {
            std::visit([this](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, ResizeViewportIntent>)            HandleResize(arg);
                else if constexpr (std::is_same_v<T, ChangeTargetIntent>)         HandleChangeTarget(arg);
                else if constexpr (std::is_same_v<T, ChangeGizmoOperationIntent>) _state.gizmoOperation = arg.op;
                else if constexpr (std::is_same_v<T, ChangeGizmoModeIntent>)      _state.gizmoMode = arg.mode;
                else if constexpr (std::is_same_v<T, ToggleSnapIntent>)           _state.useSnap = arg.useSnap;
                else if constexpr (std::is_same_v<T, ApplyGizmoTransformIntent>)  HandleGizmoTransform(arg);
                }, intent);
        }

    private:
        void HandleResize(const ResizeViewportIntent& intent) {
            if (intent.width > 0 && intent.height > 0 &&
                (_state.width != intent.width || _state.height != intent.height))
            {
                _state.width = intent.width;
                _state.height = intent.height;

                _renderApi->ResizeRenderTargets(_state.width, _state.height);
            }
        }

        void HandleChangeTarget(const ChangeTargetIntent& intent) {
            _state.currentGroup = intent.currentGroup;
            _state.currentTarget = intent.targetName;
            _state.currentView = intent.viewName;
        }

        void HandleGizmoTransform(const ApplyGizmoTransformIntent& intent) {
            if (_state.activeEntity == NULL_ENTITY) 
                return;

            glm::mat4 localTransform = intent.newWorldMatrix;
            if (_state.hasParent) {
                localTransform = glm::inverse(_state.parentWorldTransform) * intent.newWorldMatrix;
            }

            glm::vec3 translation, rotation, scale;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localTransform), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));

            _transformApi->SetEntityPosition(_state.activeEntity, translation);
            _transformApi->SetEntityRotation(_state.activeEntity, rotation);
            _transformApi->SetEntityScale(_state.activeEntity, scale);
        }
    private:
        IRenderAPI* _renderApi = nullptr;
        ISelectionAPI* _selectionApi = nullptr;
        ITransformAPI* _transformApi = nullptr;
        ViewportState _state;
    };
}