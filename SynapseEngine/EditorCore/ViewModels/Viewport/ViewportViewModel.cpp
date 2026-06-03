#include "ViewportViewModel.h"
#include <glm/gtc/type_ptr.hpp>
#include <print>

namespace Syn {

    ViewportViewModel::ViewportViewModel(IRenderAPI* renderApi, ISelectionAPI* selectionApi, ITransformAPI* transformApi, ISettingsAPI* settingsApi)
        : _renderApi(renderApi), _selectionApi(selectionApi), _transformApi(transformApi), _settingsApi(settingsApi) {}

    const ViewportState& ViewportViewModel::GetState() const {
        return _state;
    }

    void ViewportViewModel::SyncWithEngine() {
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

        if (_settingsApi) {
            SceneSettings settings = _settingsApi->GetSceneSettings();
            _state.enableDebugVisibility = settings.enableDebugVisibility;
            _state.debugVisibilityMode = static_cast<uint32_t>(settings.debugVisibilityMode);
        }
    }

    void ViewportViewModel::Dispatch(const ViewportIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ResizeViewportIntent>)            HandleResize(arg);
            else if constexpr (std::is_same_v<T, ChangeTargetIntent>)         HandleChangeTarget(arg);
            else if constexpr (std::is_same_v<T, ChangeGizmoOperationIntent>) _state.gizmoOperation = arg.op;
            else if constexpr (std::is_same_v<T, ChangeGizmoModeIntent>)      _state.gizmoMode = arg.mode;
            else if constexpr (std::is_same_v<T, ToggleSnapIntent>)           _state.useSnap = arg.useSnap;
            else if constexpr (std::is_same_v<T, ApplyGizmoTransformIntent>)  HandleGizmoTransform(arg);
            else if constexpr (std::is_same_v<T, PickEntityIntent>)           HandlePickEntity(arg);
            else if constexpr (std::is_same_v<T, ToggleDebugVisibilityIntent>)     HandleToggleDebugVisibility(arg);
            else if constexpr (std::is_same_v<T, ChangeDebugVisibilityModeIntent>) HandleChangeDebugVisibilityMode(arg);
            else if constexpr (std::is_same_v<T, ChangeSnapTranslateIntent>) _state.snapTranslate = arg.snap;
            else if constexpr (std::is_same_v<T, ChangeSnapRotateIntent>)    _state.snapAngle = arg.angle;
            else if constexpr (std::is_same_v<T, ChangeSnapScaleIntent>)     _state.snapScale = arg.scale;

            else if constexpr (std::is_same_v<T, PlaySimulationIntent>) {
                _state.simState = SimulationState::Playing;
                // TODO:  Scripts Init, Physics Init
            }
            else if constexpr (std::is_same_v<T, PauseSimulationIntent>) {
                if (_state.simState == SimulationState::Playing) {
                    _state.simState = SimulationState::Paused;
                    // TODO: Physics Pause
                }
                else if (_state.simState == SimulationState::Paused) {
                    _state.simState = SimulationState::Playing;
                }
            }
            else if constexpr (std::is_same_v<T, StopSimulationIntent>) {
                _state.simState = SimulationState::Stopped;
                // TODO: Restore original scene state
            }

            }, intent);
    }

    void ViewportViewModel::HandlePickEntity(const PickEntityIntent& intent) {
        EntityID clickedEntity = _renderApi->ReadEntityIdAtPixel(intent.x, intent.y);
        _selectionApi->SetSelectedEntity(clickedEntity);
    }

    void ViewportViewModel::HandleResize(const ResizeViewportIntent& intent) {
        if (intent.width > 0 && intent.height > 0 &&
            (_state.width != intent.width || _state.height != intent.height))
        {
            _state.width = intent.width;
            _state.height = intent.height;

            _renderApi->ResizeRenderTargets(_state.width, _state.height);
        }
    }

    void ViewportViewModel::HandleChangeTarget(const ChangeTargetIntent& intent) {
        _state.currentGroup = intent.currentGroup;
        _state.currentTarget = intent.targetName;
        _state.currentView = intent.viewName;
    }

    void ViewportViewModel::HandleGizmoTransform(const ApplyGizmoTransformIntent& intent) {
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

    void ViewportViewModel::HandleToggleDebugVisibility(const ToggleDebugVisibilityIntent& intent) {
        _state.enableDebugVisibility = intent.enabled;
        if (_settingsApi) {
            SceneSettings settings = _settingsApi->GetSceneSettings();
            settings.enableDebugVisibility = intent.enabled;
            _settingsApi->SetSceneSettings(settings);
        }
    }

    void ViewportViewModel::HandleChangeDebugVisibilityMode(const ChangeDebugVisibilityModeIntent& intent) {
        _state.debugVisibilityMode = intent.mode;
        if (_settingsApi) {
            SceneSettings settings = _settingsApi->GetSceneSettings();
            settings.debugVisibilityMode = static_cast<DebugVisibilityMode>(intent.mode);
            _settingsApi->SetSceneSettings(settings);
        }
    }

}