#include "ConvexColliderViewModel.h"

namespace Syn
{
    ConvexColliderViewModel::ConvexColliderViewModel(ISelectionApi* selectionApi, IConvexColliderApi* colliderApi)
        : _selectionApi(selectionApi), _colliderApi(colliderApi)
    {}

    const ConvexColliderState& ConvexColliderViewModel::GetState() const
    {
        return _state;
    }

    void ConvexColliderViewModel::SyncWithEngine()
    {
        if (!_selectionApi || !_colliderApi)
            return;

        EntityID activeEntity = _selectionApi->GetSelectedEntity();

        if (activeEntity != NULL_ENTITY && _colliderApi->HasConvexCollider(activeEntity))
        {
            _state.hasComponent = true;

            _state.targetLodLevel = _colliderApi->GetConvexColliderTargetLodLevel(activeEntity);

            if (!_localOffsetDrag.IsDragging())
                _state.localOffset = _colliderApi->GetConvexColliderLocalOffset(activeEntity);
        }
        else
        {
            _state.hasComponent = false;
        }
    }

    void ConvexColliderViewModel::Dispatch(const ConvexColliderIntent& intent)
    {
        std::visit([this](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, SetConvexColliderTargetLodLevelIntent>) HandleSetTargetLodLevel(arg);
                else if constexpr (std::is_same_v<T, SetConvexColliderLocalOffsetIntent>) HandleSetLocalOffset(arg); }, intent);
    }

    void ConvexColliderViewModel::HandleSetTargetLodLevel(const SetConvexColliderTargetLodLevelIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _state.targetLodLevel = intent.targetLodLevel;
        _colliderApi->SetConvexColliderTargetLodLevel(activeEntity, intent.targetLodLevel);
    }

    void ConvexColliderViewModel::HandleSetLocalOffset(const SetConvexColliderLocalOffsetIntent& intent)
    {
        EntityID activeEntity = _selectionApi->GetSelectedEntity();
        if (activeEntity == NULL_ENTITY) return;

        _localOffsetDrag.Handle(intent.isDragging, intent.localOffset, _state.localOffset,
            [&](const glm::vec3& v) { _colliderApi->SetConvexColliderLocalOffset(activeEntity, v); },
            [&](const glm::vec3& s, const glm::vec3& e) { return std::make_shared<ChangeConvexColliderLocalOffsetCommand>(_colliderApi, activeEntity, s, e); });
    }
}