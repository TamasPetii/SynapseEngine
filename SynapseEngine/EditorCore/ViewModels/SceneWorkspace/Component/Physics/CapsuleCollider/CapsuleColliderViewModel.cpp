#include "CapsuleColliderViewModel.h"

namespace Syn
{
    CapsuleColliderViewModel::CapsuleColliderViewModel(ISelectionApi* selectionApi, ICapsuleColliderApi* colliderApi)
        : _selectionApi(selectionApi), _colliderApi(colliderApi)
    {}

    const CapsuleColliderState& CapsuleColliderViewModel::GetState() const
    {
        return _state;
    }

    void CapsuleColliderViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_colliderApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _colliderApi->HasCapsuleCollider(activeEntity))
        {
            _state.hasComponent = true;

            if (!_radiusDrag.IsDragging())
                _state.radius = _colliderApi->GetCapsuleColliderRadius(activeEntity);
            if (!_halfHeightDrag.IsDragging())
                _state.halfHeight = _colliderApi->GetCapsuleColliderHalfHeight(activeEntity);
            if (!_localOffsetDrag.IsDragging())
                _state.localOffset = _colliderApi->GetCapsuleColliderLocalOffset(activeEntity);
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void CapsuleColliderViewModel::Dispatch(const CapsuleColliderIntent& intent)
    {
        std::visit([this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetCapsuleColliderRadiusIntent>)         HandleSetRadius(arg);
                else if constexpr (std::is_same_v<T, SetCapsuleColliderHalfHeightIntent>)     HandleSetHalfHeight(arg);
                else if constexpr (std::is_same_v<T, SetCapsuleColliderLocalOffsetIntent>)    HandleSetLocalOffset(arg); }, intent);
    }

    void CapsuleColliderViewModel::HandleSetRadius(const SetCapsuleColliderRadiusIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _radiusDrag.Handle(intent.isDragging, intent.radius, _state.radius,
            [&](const float& v) { _colliderApi->SetCapsuleColliderRadius(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeCapsuleColliderRadiusCommand>(_colliderApi, activeEntity, s, e); });
    }

    void CapsuleColliderViewModel::HandleSetHalfHeight(const SetCapsuleColliderHalfHeightIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _halfHeightDrag.Handle(intent.isDragging, intent.halfHeight, _state.halfHeight,
            [&](const float& v) { _colliderApi->SetCapsuleColliderHalfHeight(activeEntity, v); },
            [&](const float& s, const float& e) { return std::make_shared<ChangeCapsuleColliderHalfHeightCommand>(_colliderApi, activeEntity, s, e); });
    }

    void CapsuleColliderViewModel::HandleSetLocalOffset(const SetCapsuleColliderLocalOffsetIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _localOffsetDrag.Handle(intent.isDragging, intent.localOffset, _state.localOffset,
            [&](const glm::vec3& v) { _colliderApi->SetCapsuleColliderLocalOffset(activeEntity, v); },
            [&](const glm::vec3& s, const glm::vec3& e) { return std::make_shared<ChangeCapsuleColliderLocalOffsetCommand>(_colliderApi, activeEntity, s, e); });
    }
}