#include "ModelViewportViewModel.h"
#include <glm/gtc/type_ptr.hpp>

namespace Syn {

    ModelViewportViewModel::ModelViewportViewModel(IRenderApi* renderApi, ISelectionApi* selectionApi, ITransformApi* transformApi, ISettingsApi* settingsApi, IModelApi* modelApi)
        : _renderApi(renderApi), _selectionApi(selectionApi), _transformApi(transformApi), _settingsApi(settingsApi), _modelApi(modelApi) {}

    const ModelViewportState& ModelViewportViewModel::GetState() const {
        return _state;
    }

    void ModelViewportViewModel::SyncWithEngine() {
        if (!_renderApi) return;

        _state.textureId = _renderApi->GetViewportTexture(_state.currentGroup, _state.currentTarget, _state.currentView);
        _state.cameraView = _renderApi->GetEditorCameraView();
        _state.cameraProj = _renderApi->GetEditorCameraProjection();

        if (_modelApi) {
            auto [mId, nIdx] = _modelApi->GetSelected();
            _state.activeModelId = mId;
            _state.activeNodeIndex = nIdx;
        }

        _state.activeEntity = _selectionApi->GetSelectedEntity();
        if (_state.activeEntity != NULL_ENTITY) {
            _state.entityWorldTransform = _transformApi->GetEntityWorldMatrix(_state.activeEntity);
        }

        if (_settingsApi) {
            SceneSettings settings = _settingsApi->GetSceneSettings();
            _state.enableDebugVisibility = settings.debug.enableDebugVisibility;
            _state.debugVisibilityMode = static_cast<uint32_t>(settings.debug.debugVisibilityMode);
        }
    }

    void ModelViewportViewModel::Dispatch(const ModelViewportIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ResizeModelViewportIntent>)            HandleResize(arg);
            else if constexpr (std::is_same_v<T, ChangeModelTargetIntent>)         HandleChangeTarget(arg);
            else if constexpr (std::is_same_v<T, ChangeModelGizmoOperationIntent>) _state.gizmoOperation = arg.op;
            else if constexpr (std::is_same_v<T, ChangeModelGizmoModeIntent>)      _state.gizmoMode = arg.mode;
            else if constexpr (std::is_same_v<T, ToggleModelSnapIntent>)           _state.useSnap = arg.useSnap;
            else if constexpr (std::is_same_v<T, ApplyModelGizmoTransformIntent>)  HandleGizmoTransform(arg);
            else if constexpr (std::is_same_v<T, PickMeshIntent>)                  HandlePickMesh(arg);
            else if constexpr (std::is_same_v<T, ToggleModelDebugVisibilityIntent>)     HandleToggleDebugVisibility(arg);
            else if constexpr (std::is_same_v<T, ChangeModelDebugVisibilityModeIntent>) HandleChangeDebugVisibilityMode(arg);
            else if constexpr (std::is_same_v<T, ChangeModelSnapTranslateIntent>) _state.snapTranslate = arg.snap;
            else if constexpr (std::is_same_v<T, ChangeModelSnapRotateIntent>)    _state.snapAngle = arg.angle;
            else if constexpr (std::is_same_v<T, ChangeModelSnapScaleIntent>)     _state.snapScale = arg.scale;
            }, intent);
    }

    void ModelViewportViewModel::HandlePickMesh(const PickMeshIntent& intent) {
        auto [entityId, meshIndex] = _renderApi->ReadEntityAndMeshIdAtPixel(intent.x, intent.y);

        if (entityId != NULL_ENTITY) {
            _selectionApi->SetSelectedEntity(entityId);

            if (_modelApi) {
                auto [currentModel, currentNode] = _modelApi->GetSelected();
                if (currentModel != INVALID_MODEL_ID) {
                    _modelApi->SetSelected(currentModel, static_cast<int32_t>(meshIndex));
                }
            }
        }
    }

    void ModelViewportViewModel::HandleResize(const ResizeModelViewportIntent& intent) {
        if (intent.width > 0 && intent.height > 0 &&
            (_state.width != intent.width || _state.height != intent.height))
        {
            _state.width = intent.width;
            _state.height = intent.height;
            _renderApi->ResizeRenderTargets(_state.width, _state.height);
        }
    }

    void ModelViewportViewModel::HandleChangeTarget(const ChangeModelTargetIntent& intent) {
        _state.currentGroup = intent.currentGroup;
        _state.currentTarget = intent.targetName;
        _state.currentView = intent.viewName;
    }

    void ModelViewportViewModel::HandleGizmoTransform(const ApplyModelGizmoTransformIntent& intent) {
        if (_state.activeEntity == NULL_ENTITY)
            return;

        glm::vec3 translation, rotation, scale;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(intent.newWorldMatrix), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));

        _transformApi->SetEntityPosition(_state.activeEntity, translation);
        _transformApi->SetEntityRotation(_state.activeEntity, rotation);
        _transformApi->SetEntityScale(_state.activeEntity, scale);
    }

    void ModelViewportViewModel::HandleToggleDebugVisibility(const ToggleModelDebugVisibilityIntent& intent) {
        _state.enableDebugVisibility = intent.enabled;
        if (_settingsApi) {
            SceneSettings settings = _settingsApi->GetSceneSettings();
            settings.debug.enableDebugVisibility = intent.enabled;
            _settingsApi->SetSceneSettings(settings);
        }
    }

    void ModelViewportViewModel::HandleChangeDebugVisibilityMode(const ChangeModelDebugVisibilityModeIntent& intent) {
        _state.debugVisibilityMode = intent.mode;
        if (_settingsApi) {
            SceneSettings settings = _settingsApi->GetSceneSettings();
            settings.debug.debugVisibilityMode = static_cast<DebugVisibilityMode>(intent.mode);
            _settingsApi->SetSceneSettings(settings);
        }
    }

}