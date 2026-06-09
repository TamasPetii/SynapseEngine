#include "SelectionApiImpl.h"

namespace Syn {
    EntityID SelectionApiImpl::GetSelectedEntity() const {
        auto scene = _sceneManager->GetActiveScene();
        return scene ? scene->GetSelectedEntity() : NULL_ENTITY;
    }

    void SelectionApiImpl::SetSelectedEntity(EntityID entity) {
        if (auto scene = _sceneManager->GetActiveScene()) {
            scene->SetSelectedEntity(entity);
        }
    }
}