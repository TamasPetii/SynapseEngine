#include "TagApiImpl.h"
#include "../EditorApiUtils.h"
#include "Engine/Component/Core/TagComponent.h"

namespace Syn {
    std::string TagApiImpl::GetEntityTag(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TagComponent>(_sceneManager, entity, 
            [](const auto& c) { return c.tag; }, std::string("Untagged"));
    }

    void TagApiImpl::SetEntityTag(EntityID entity, const std::string& tag) {
        EditorApiUtils::ModifyComponent<TagComponent>(_sceneManager, entity, 
            [&](auto& c, auto pool) { c.tag = tag; });
    }

    std::string TagApiImpl::GetEntityName(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TagComponent>(_sceneManager, entity, 
            [](const auto& c) { return c.name; }, "Entity " + std::to_string(entity));
    }

    void TagApiImpl::SetEntityName(EntityID entity, const std::string& name) {
        EditorApiUtils::ModifyComponent<TagComponent>(_sceneManager, entity, 
            [&](auto& c, auto pool) { c.name = name; });
    }

    bool TagApiImpl::IsEntityEnabled(EntityID entity) const {
        return EditorApiUtils::ReadComponent<TagComponent>(_sceneManager, entity, 
            [](const auto& c) { return c.localEnabled; }, true);
    }

    void TagApiImpl::SetEntityEnabled(EntityID entity, bool enabled) {
        EditorApiUtils::ModifyComponent<TagComponent>(_sceneManager, entity, 
            [&](auto& c, auto pool) { c.localEnabled = enabled; });
    }
}