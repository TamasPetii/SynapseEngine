#pragma once
#include "EditorCore/Api/IHierarchyApi.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class HierarchyApiImpl : public IHierarchyApi {
    public:
        HierarchyApiImpl(SceneManager* sm) : _sceneManager(sm) {}

        uint64_t GetVersion() const override;
        std::vector<EntityID> GetRootEntities() const override;
        std::vector<EntityID> GetChildren(EntityID entity) const override;
        std::string GetEntityIcon(EntityID entity) const override;
        bool HasChildren(EntityID entity) const override;
        EntityID GetParent(EntityID entity) const override;
        void SetParent(EntityID child, EntityID parent) override;
        EntityID CreateEntity(const std::string& name, EntityID parent = NULL_ENTITY) override;
        void DestroyEntity(EntityID entity) override;
    private:
        SceneManager* _sceneManager;
    };
}