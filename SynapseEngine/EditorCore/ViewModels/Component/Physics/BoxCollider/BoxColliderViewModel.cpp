#include "BoxColliderViewModel.h"

namespace Syn
{
    BoxColliderViewModel::BoxColliderViewModel(ISelectionApi* selectionApi, IBoxColliderApi* colliderApi)
        : _selectionApi(selectionApi), _colliderApi(colliderApi)
    {}

    const BoxColliderState& BoxColliderViewModel::GetState() const
    {
        return _state;
    }

    void BoxColliderViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_colliderApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _colliderApi->HasBoxCollider(activeEntity))
        {
            _state.hasComponent = true;

            if (!_halfExtentsDrag.IsDragging())
                _state.halfExtents = _colliderApi->GetBoxColliderHalfExtents(activeEntity);
            if (!_localOffsetDrag.IsDragging())
                _state.localOffset = _colliderApi->GetBoxColliderLocalOffset(activeEntity);
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void BoxColliderViewModel::Dispatch(const BoxColliderIntent& intent)
    {
        std::visit([this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetBoxColliderHalfExtentsIntent>) HandleSetHalfExtents(arg);
                else if constexpr (std::is_same_v<T, SetBoxColliderLocalOffsetIntent>) HandleSetLocalOffset(arg); }, intent);
    }

    void BoxColliderViewModel::HandleSetHalfExtents(const SetBoxColliderHalfExtentsIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _halfExtentsDrag.Handle(intent.isDragging, intent.halfExtents, _state.halfExtents,
            [&](const glm::vec3& v) { _colliderApi->SetBoxColliderHalfExtents(activeEntity, v); },
            [&](const glm::vec3& s, const glm::vec3& e) { return std::make_shared<ChangeBoxColliderHalfExtentsCommand>(_colliderApi, activeEntity, s, e); });
    }

    void BoxColliderViewModel::HandleSetLocalOffset(const SetBoxColliderLocalOffsetIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _localOffsetDrag.Handle(intent.isDragging, intent.localOffset, _state.localOffset,
            [&](const glm::vec3& v) { _colliderApi->SetBoxColliderLocalOffset(activeEntity, v); },
            [&](const glm::vec3& s, const glm::vec3& e) { return std::make_shared<ChangeBoxColliderLocalOffsetCommand>(_colliderApi, activeEntity, s, e); });
    }
}