#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Interaction/DragInteraction.h"
#include "RigidBodyState.h"
#include "RigidBodyIntent.h"
#include "RigidBodyCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IRigidBodyApi.h"

namespace Syn {
    class RigidBodyViewModel : public IViewModel<RigidBodyState, RigidBodyIntent> {
    public:
        RigidBodyViewModel(ISelectionApi* selectionApi, IRigidBodyApi* rigidBodyApi);
        ~RigidBodyViewModel() override = default;

        const RigidBodyState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const RigidBodyIntent& intent) override;

    private:
        void HandleSetMotionType(const SetRigidBodyMotionTypeIntent& intent);
        void HandleSetMass(const SetRigidBodyMassIntent& intent);
        void HandleSetFriction(const SetRigidBodyFrictionIntent& intent);
        void HandleSetRestitution(const SetRigidBodyRestitutionIntent& intent);
        void HandleSetLayer(const SetRigidBodyLayerIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        IRigidBodyApi* _rigidBodyApi = nullptr;
        RigidBodyState _state;

        DragInteraction<float> _massDrag;
        DragInteraction<float> _frictionDrag;
        DragInteraction<float> _restitutionDrag;
    };
}