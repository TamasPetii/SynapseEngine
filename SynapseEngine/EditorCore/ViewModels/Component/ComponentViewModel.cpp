#include "ComponentViewModel.h"

namespace Syn 
{
    ComponentViewModel::ComponentViewModel(
        ISelectionApi* selectionApi,
        ITagApi* tagApi,
        ITransformApi* transformApi,
        IHierarchyApi* hierarchyApi, 
        IDirectionLightApi* directionLightApi,
        IPointLightApi* pointLightApi,
        ISpotLightApi* spotLightApi,
        ICameraApi* cameraApi,
        IBoxColliderApi* boxColliderApi,
        ISphereColliderApi* sphereColliderApi,
        ICapsuleColliderApi* capsuleColliderApi,
        IConvexColliderApi* convexColliderApi,
        IMeshColliderApi* meshColliderApi,
        IRigidBodyApi* rigidBodyApi,
		IModelComponentApi* modelComponentApi,
		IAnimationApi* animationApi
    )
        : 
        _selectionApi(selectionApi),
        _tagViewModel(selectionApi, tagApi),
        _transformViewModel(selectionApi, transformApi, hierarchyApi),
        _directionLightViewModel(selectionApi, directionLightApi),
        _pointLightViewModel(selectionApi, pointLightApi),
        _spotLightViewModel(selectionApi, spotLightApi),
        _cameraViewModel(selectionApi, cameraApi),
        _boxColliderViewModel(selectionApi, boxColliderApi),
        _sphereColliderViewModel(selectionApi, sphereColliderApi),
        _capsuleColliderViewModel(selectionApi, capsuleColliderApi),
        _convexColliderViewModel(selectionApi, convexColliderApi),
        _meshColliderViewModel(selectionApi, meshColliderApi),
        _rigidBodyViewModel(selectionApi, rigidBodyApi),
		_modelComponentViewModel(selectionApi, modelComponentApi),
		_animationViewModel(selectionApi, animationApi)
    {}

    const ComponentState& ComponentViewModel::GetState() const {
        return _state;
    }

    void ComponentViewModel::SyncWithEngine() {
        if (!_selectionApi) return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        _state.activeEntityId = activeEntity;
        _state.hasSelection = (activeEntity != NULL_ENTITY);

        if (_state.hasSelection) {
            _tagViewModel.SyncWithEngine();
            _transformViewModel.SyncWithEngine();
            _directionLightViewModel.SyncWithEngine();
            _pointLightViewModel.SyncWithEngine();
            _spotLightViewModel.SyncWithEngine();
            _cameraViewModel.SyncWithEngine();
            _boxColliderViewModel.SyncWithEngine();
            _sphereColliderViewModel.SyncWithEngine();
            _capsuleColliderViewModel.SyncWithEngine();
            _convexColliderViewModel.SyncWithEngine();
            _meshColliderViewModel.SyncWithEngine();
            _rigidBodyViewModel.SyncWithEngine();
			_modelComponentViewModel.SyncWithEngine();
			_animationViewModel.SyncWithEngine();
        }
    }

    void ComponentViewModel::Dispatch(const ComponentIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, TagIntent>) {
                _tagViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, TransformIntent>) {
                _transformViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, DirectionLightIntent>) {
                _directionLightViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, PointLightIntent>) {
                _pointLightViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, SpotLightIntent>) {
                _spotLightViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, CameraIntent>) {
                _cameraViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, BoxColliderIntent>) {
                _boxColliderViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, SphereColliderIntent>) {
                _sphereColliderViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, CapsuleColliderIntent>) {
                _capsuleColliderViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, ConvexColliderIntent>) {
                _convexColliderViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, MeshColliderIntent>) {
                _meshColliderViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, RigidBodyIntent>) {
                _rigidBodyViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, ModelComponentIntent>) {
                _modelComponentViewModel.Dispatch(arg);
            }
            else if constexpr (std::is_same_v<T, AnimationIntent>) {
                _animationViewModel.Dispatch(arg);
            }
        }, intent);
    }
}