#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "Core/Tag/TagViewModel.h"
#include "Core/Transform/TransformViewModel.h"
#include "ComponentState.h"
#include "ComponentIntent.h"

namespace Syn {
    class ComponentViewModel : public IViewModel<ComponentState, ComponentIntent> {
    public:
        ComponentViewModel(ISelectionApi* selectionApi, ITagApi* tagApi, ITransformApi* transformApi);
        ~ComponentViewModel() override = default;

        const ComponentState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const ComponentIntent& intent) override;

        TagViewModel& GetTagVM();
        TransformViewModel& GetTransformVM();
    private:
        ISelectionApi* _selectionApi = nullptr;
        ComponentState _state;

        TagViewModel _tagVM;
        TransformViewModel _transformVM;
    };
}