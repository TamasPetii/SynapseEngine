#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "ComponentState.h"
#include "ComponentIntent.h"

#include "Core/Tag/TagViewModel.h"
#include "Core/Transform/TransformViewModel.h"
#include "Light/DirectionLight/DirectionLightViewModel.h"
#include "Light/PointLight/PointLightViewModel.h"
#include "Light/SpotLight/SpotLightViewModel.h"
#include "Core/Camera/CameraViewModel.h"

#include "Physics/BoxCollider/BoxColliderViewModel.h"
#include "Physics/SphereCollider/SphereColliderViewModel.h"
#include "Physics/CapsuleCollider/CapsuleColliderViewModel.h"
#include "Physics/ConvexCollider/ConvexColliderViewModel.h"
#include "Physics/MeshCollider/MeshColliderViewModel.h"
#include "Physics/RigidBody/RigidBodyViewModel.h"

#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ITagApi.h"
#include "EditorCore/Api/ITransformApi.h"
#include "EditorCore/Api/IDirectionLightApi.h"
#include "EditorCore/Api/IPointLightApi.h"
#include "EditorCore/Api/ICameraApi.h"
#include "EditorCore/Api/IBoxColliderApi.h"
#include "EditorCore/Api/ISphereColliderApi.h"
#include "EditorCore/Api/ICapsuleColliderApi.h"
#include "EditorCore/Api/IConvexColliderApi.h"
#include "EditorCore/Api/IMeshColliderApi.h"
#include "EditorCore/Api/IRigidBodyApi.h"

namespace Syn {
    class ComponentViewModel : public IViewModel<ComponentState, ComponentIntent> {
    public:
        ComponentViewModel(
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
            IRigidBodyApi* rigidBodyApi
            );

        ~ComponentViewModel() override = default;

        const ComponentState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const ComponentIntent& intent) override;

		TagViewModel& GetTagViewModel() { return _tagViewModel; }
		TransformViewModel& GetTransformViewModel() { return _transformViewModel; }
		DirectionLightViewModel& GetDirectionLightViewModel() { return _directionLightViewModel; }
        PointLightViewModel& GetPointLightViewModel() { return _pointLightViewModel; }
        SpotLightViewModel& GetSpotLightViewModel() { return _spotLightViewModel; }
        CameraViewModel& GetCameraViewModel() { return _cameraViewModel; }
        BoxColliderViewModel& GetBoxColliderViewModel() { return _boxColliderViewModel; }
        SphereColliderViewModel& GetSphereColliderViewModel() { return _sphereColliderViewModel; }
        CapsuleColliderViewModel& GetCapsuleColliderViewModel() { return _capsuleColliderViewModel; }
        ConvexColliderViewModel& GetConvexColliderViewModel() { return _convexColliderViewModel; }
        MeshColliderViewModel& GetMeshColliderViewModel() { return _meshColliderViewModel; }
        RigidBodyViewModel& GetRigidBodyViewModel() { return _rigidBodyViewModel; }
    private:
		ISelectionApi* _selectionApi = nullptr;
        ComponentState _state;

        TagViewModel _tagViewModel;
        CameraViewModel _cameraViewModel;
        TransformViewModel _transformViewModel;
        DirectionLightViewModel _directionLightViewModel;
        PointLightViewModel _pointLightViewModel;
        SpotLightViewModel _spotLightViewModel;

        BoxColliderViewModel _boxColliderViewModel;
        SphereColliderViewModel _sphereColliderViewModel;
        CapsuleColliderViewModel _capsuleColliderViewModel;
        ConvexColliderViewModel _convexColliderViewModel;
        MeshColliderViewModel _meshColliderViewModel;
        RigidBodyViewModel _rigidBodyViewModel;
    };
}