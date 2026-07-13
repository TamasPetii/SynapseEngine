#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Interaction/DragInteraction.h"
#include "SphereColliderState.h"
#include "SphereColliderIntent.h"
#include "SphereColliderCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/ISphereColliderApi.h"

namespace Syn {
    class SphereColliderViewModel : public IViewModel<SphereColliderState, SphereColliderIntent> {
    public:
        SphereColliderViewModel(ISelectionApi* selectionApi, ISphereColliderApi* colliderApi);
        ~SphereColliderViewModel() override = default;

        const SphereColliderState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const SphereColliderIntent& intent) override;

    private:
        void HandleSetRadius(const SetSphereColliderRadiusIntent& intent);
        void HandleSetLocalOffset(const SetSphereColliderLocalOffsetIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        ISphereColliderApi* _colliderApi = nullptr;
        SphereColliderState _state;

        DragInteraction<float> _radiusDrag;
        DragInteraction<glm::vec3> _localOffsetDrag;
    };
}