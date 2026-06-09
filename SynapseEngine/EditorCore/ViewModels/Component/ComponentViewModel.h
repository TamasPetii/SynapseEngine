#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "ComponentState.h"
#include "ComponentIntent.h"

#include "Core/Tag/TagViewModel.h"
#include "Core/Transform/TransformViewModel.h"
#include "Light/DirectionLight/DirectionLightViewModel.h"

#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ITagApi.h"
#include "EditorCore/Api/ITransformApi.h"
#include "EditorCore/Api/IDirectionLightApi.h"

namespace Syn {
    class ComponentViewModel : public IViewModel<ComponentState, ComponentIntent> {
    public:
        ComponentViewModel(ISelectionApi* selectionApi, ITagApi* tagApi, ITransformApi* transformApi, IDirectionLightApi* directionLightApi, IHierarchyApi* hierarchyApi);
        ~ComponentViewModel() override = default;

        const ComponentState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const ComponentIntent& intent) override;

		TagViewModel& GetTagViewModel() { return _tagViewModel; }
		TransformViewModel& GetTransformViewModel() { return _transformViewModel; }
		DirectionLightViewModel& GetDirectionLightViewModel() { return _directionLightViewModel; }
    private:
		ISelectionApi* _selectionApi = nullptr;
        ComponentState _state;

        TagViewModel _tagViewModel;;
        TransformViewModel _transformViewModel;
        DirectionLightViewModel _directionLightViewModel;
    };
}