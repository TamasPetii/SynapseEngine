#include "EditorApiImpl.h"

namespace Syn {
    EntityID EditorApiImpl::GetSelectedEntity() const {
        return _selectedEntity;
    }

    void EditorApiImpl::SetSelectedEntity(EntityID entity) {
        _selectedEntity = entity;
    }
};