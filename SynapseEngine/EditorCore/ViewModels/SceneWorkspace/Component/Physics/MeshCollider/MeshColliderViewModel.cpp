#include "MeshColliderViewModel.h"

namespace Syn
{
    MeshColliderViewModel::MeshColliderViewModel(ISelectionApi* selectionApi, IMeshColliderApi* colliderApi)
        : _selectionApi(selectionApi), _colliderApi(colliderApi)
    {}

    const MeshColliderState& MeshColliderViewModel::GetState() const
    {
        return _state;
    }

    void MeshColliderViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_colliderApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _colliderApi->HasMeshCollider(activeEntity))
        {
            _state.hasComponent = true;

            _state.targetLodLevel = _colliderApi->GetMeshColliderTargetLodLevel(activeEntity);

            if (!_localOffsetDrag.IsDragging())
                _state.localOffset = _colliderApi->GetMeshColliderLocalOffset(activeEntity);
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void MeshColliderViewModel::Dispatch(const MeshColliderIntent& intent)
    {
        std::visit([this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetMeshColliderTargetLodLevelIntent>) HandleSetTargetLodLevel(arg);
                else if constexpr (std::is_same_v<T, SetMeshColliderLocalOffsetIntent>) HandleSetLocalOffset(arg); }, intent);
    }

    void MeshColliderViewModel::HandleSetTargetLodLevel(const SetMeshColliderTargetLodLevelIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _state.targetLodLevel = intent.targetLodLevel;
        _colliderApi->SetMeshColliderTargetLodLevel(activeEntity, intent.targetLodLevel);
    }

    void MeshColliderViewModel::HandleSetLocalOffset(const SetMeshColliderLocalOffsetIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _localOffsetDrag.Handle(intent.isDragging, intent.localOffset, _state.localOffset,
            [&](const glm::vec3& v) { _colliderApi->SetMeshColliderLocalOffset(activeEntity, v); },
            [&](const glm::vec3& s, const glm::vec3& e) { return std::make_shared<ChangeMeshColliderLocalOffsetCommand>(_colliderApi, activeEntity, s, e); });
    }
}