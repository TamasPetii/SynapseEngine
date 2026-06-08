#include "EditorApiImpl.h"

namespace Syn {
    EntityID EditorApiImpl::GetSelectedEntity() const {
        auto scene = _sceneManager->GetActiveScene();

        if (scene == nullptr)
            return NULL_ENTITY;

        return scene->GetSelectedEntity();
    }

    void EditorApiImpl::SetSelectedEntity(EntityID entity) {
        auto scene = _sceneManager->GetActiveScene();

        if (scene == nullptr)
            return;

        scene->SetSelectedEntity(entity);
    }
};