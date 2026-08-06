#include "AnimationViewportViewModel.h"
#include <glm/gtc/type_ptr.hpp>

namespace Syn {

    AnimationViewportViewModel::AnimationViewportViewModel(IRenderApi* renderApi, ISelectionApi* selectionApi, ITransformApi* transformApi, ISettingsApi* settingsApi, IAnimationApi* animationApi)
        : _renderApi(renderApi), _selectionApi(selectionApi), _transformApi(transformApi), _settingsApi(settingsApi), _animationApi(animationApi) {}

    const AnimationViewportState& AnimationViewportViewModel::GetState() const {
        return _state;
    }

    void AnimationViewportViewModel::SyncWithEngine() {
        if (!_renderApi) return;

        _state.textureId = _renderApi->GetViewportTexture(_state.currentGroup, _state.currentTarget, _state.currentView);
        _state.cameraView = _renderApi->GetEditorCameraView();
        _state.cameraProj = _renderApi->GetEditorCameraProjection();

        if (_animationApi) {
            _state.activeAnimationId = _animationApi->GetSelected();
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

    void AnimationViewportViewModel::Dispatch(const AnimationViewportIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ResizeAnimationViewportIntent>)            HandleResize(arg);
            else if constexpr (std::is_same_v<T, ChangeAnimationTargetIntent>)         HandleChangeTarget(arg);
            else if constexpr (std::is_same_v<T, ChangeAnimationGizmoOperationIntent>) _state.gizmoOperation = arg.op;
            else if constexpr (std::is_same_v<T, ChangeAnimationGizmoModeIntent>)      _state.gizmoMode = arg.mode;
            else if constexpr (std::is_same_v<T, ToggleAnimationSnapIntent>)           _state.useSnap = arg.useSnap;
            else if constexpr (std::is_same_v<T, ApplyAnimationGizmoTransformIntent>)  HandleGizmoTransform(arg);
            else if constexpr (std::is_same_v<T, PickAnimationMeshIntent>)             HandlePickMesh(arg);
            else if constexpr (std::is_same_v<T, ToggleAnimationDebugVisibilityIntent>)     HandleToggleDebugVisibility(arg);
            else if constexpr (std::is_same_v<T, ChangeAnimationDebugVisibilityModeIntent>) HandleChangeDebugVisibilityMode(arg);
            else if constexpr (std::is_same_v<T, ChangeAnimationSnapTranslateIntent>) _state.snapTranslate = arg.snap;
            else if constexpr (std::is_same_v<T, ChangeAnimationSnapRotateIntent>)    _state.snapAngle = arg.angle;
            else if constexpr (std::is_same_v<T, ChangeAnimationSnapScaleIntent>)     _state.snapScale = arg.scale;
            }, intent);
    }

    void AnimationViewportViewModel::HandlePickMesh(const PickAnimationMeshIntent& intent) {
        auto [entityId, meshIndex] = _renderApi->ReadEntityAndMeshIdAtPixel(intent.x, intent.y);

        if (entityId != NULL_ENTITY) {
            _selectionApi->SetSelectedEntity(entityId);
        }
    }

    void AnimationViewportViewModel::HandleResize(const ResizeAnimationViewportIntent& intent) {
        if (intent.width > 0 && intent.height > 0 &&
            (_state.width != intent.width || _state.height != intent.height))
        {
            _state.width = intent.width;
            _state.height = intent.height;
            _renderApi->ResizeRenderTargets(_state.width, _state.height);
        }
    }

    void AnimationViewportViewModel::HandleChangeTarget(const ChangeAnimationTargetIntent& intent) {
        _state.currentGroup = intent.currentGroup;
        _state.currentTarget = intent.targetName;
        _state.currentView = intent.viewName;
    }

    void AnimationViewportViewModel::HandleGizmoTransform(const ApplyAnimationGizmoTransformIntent& intent) {
        if (_state.activeEntity == NULL_ENTITY)
            return;

        glm::vec3 translation, rotation, scale;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(intent.newWorldMatrix), glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));

        _transformApi->SetEntityPosition(_state.activeEntity, translation);
        _transformApi->SetEntityRotation(_state.activeEntity, rotation);
        _transformApi->SetEntityScale(_state.activeEntity, scale);
    }

    void AnimationViewportViewModel::HandleToggleDebugVisibility(const ToggleAnimationDebugVisibilityIntent& intent) {
        _state.enableDebugVisibility = intent.enabled;
        if (_settingsApi) {
            SceneSettings settings = _settingsApi->GetSceneSettings();
            settings.debug.enableDebugVisibility = intent.enabled;
            _settingsApi->SetSceneSettings(settings);
        }
    }

    void AnimationViewportViewModel::HandleChangeDebugVisibilityMode(const ChangeAnimationDebugVisibilityModeIntent& intent) {
        _state.debugVisibilityMode = intent.mode;
        if (_settingsApi) {
            SceneSettings settings = _settingsApi->GetSceneSettings();
            settings.debug.debugVisibilityMode = static_cast<DebugVisibilityMode>(intent.mode);
            _settingsApi->SetSceneSettings(settings);
        }
    }

}