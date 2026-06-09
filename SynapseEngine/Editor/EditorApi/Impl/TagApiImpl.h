#pragma once
#include "EditorCore/Api/ITagApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class TagApiImpl : public ITagApi {
    public:
        TagApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        std::string GetEntityName(EntityID entity) const override;
        void SetEntityName(EntityID entity, const std::string& name) override;
        std::string GetEntityTag(EntityID entity) const override;
        void SetEntityTag(EntityID entity, const std::string& tag) override;
        bool IsEntityEnabled(EntityID entity) const override;
        void SetEntityEnabled(EntityID entity, bool enabled) override;
    private:
        SceneManager* _sceneManager;
    };
}