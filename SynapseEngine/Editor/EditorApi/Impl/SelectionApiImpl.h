#pragma once
#include "EditorCore/Api/ISelectionApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class SelectionApiImpl : public ISelectionApi {
    public:
        SelectionApiImpl(SceneManager* sm) : _sceneManager(sm) {}
        EntityID GetSelectedEntity() const override;
        void SetSelectedEntity(EntityID entity) override;
    private:
        SceneManager* _sceneManager;
    };
}