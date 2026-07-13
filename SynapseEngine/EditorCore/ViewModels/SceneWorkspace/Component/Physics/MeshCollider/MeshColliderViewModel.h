#pragma once
#include "EditorCore/ViewModels/IViewModel.h"
#include "EditorCore/Interaction/DragInteraction.h"
#include "MeshColliderState.h"
#include "MeshColliderIntent.h"
#include "MeshColliderCommands.h"
#include "EditorCore/Api/ISelectionApi.h"
#include "EditorCore/Api/IMeshColliderApi.h"

namespace Syn {
    class MeshColliderViewModel : public IViewModel<MeshColliderState, MeshColliderIntent> {
    public:
        MeshColliderViewModel(ISelectionApi* selectionApi, IMeshColliderApi* colliderApi);
        ~MeshColliderViewModel() override = default;

        const MeshColliderState& GetState() const override;
        void SyncWithEngine() override;
        void Dispatch(const MeshColliderIntent& intent) override;

    private:
        void HandleSetTargetLodLevel(const SetMeshColliderTargetLodLevelIntent& intent);
        void HandleSetLocalOffset(const SetMeshColliderLocalOffsetIntent& intent);

    private:
        ISelectionApi* _selectionApi = nullptr;
        IMeshColliderApi* _colliderApi = nullptr;
        MeshColliderState _state;

        DragInteraction<glm::vec3> _localOffsetDrag;
    };
}