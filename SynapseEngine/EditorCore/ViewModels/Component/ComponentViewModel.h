#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "ComponentState.h"
#include "ComponentIntent.h"

#include "Core/Tag/TagViewModel.h"
#include "Core/Transform/TransformViewModel.h"
#include "Light/DirectionLight/DirectionLightViewModel.h"
#include "Light/PointLight/PointLightViewModel.h"
#include "Light/SpotLight/SpotLightViewModel.h"

#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ITagApi.h"
#include "EditorCore/Api/ITransformApi.h"
#include "EditorCore/Api/IDirectionLightApi.h"
#include "EditorCore/Api/IPointLightApi.h"

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
            ISpotLightApi* spotLightApi
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
    private:
		ISelectionApi* _selectionApi = nullptr;
        ComponentState _state;

        TagViewModel _tagViewModel;;
        TransformViewModel _transformViewModel;
        DirectionLightViewModel _directionLightViewModel;
        PointLightViewModel _pointLightViewModel;
        SpotLightViewModel _spotLightViewModel;
    };
}