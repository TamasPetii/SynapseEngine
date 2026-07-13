#pragma once
#include "EditorCore/Api/IHierarchyApi.h"
#include "Engine/Scene/SceneManager.h"


namespace Syn {
    class HierarchyApiImpl : public IHierarchyApi {
    public:
        HierarchyApiImpl(SceneManager* sm, ModelManager* modelManager) : _sceneManager(sm), _modelManager(modelManager) {}

        uint64_t GetVersion() const override;
        std::vector<EntityID> GetRootEntities() const override;
        std::vector<EntityID> GetChildren(EntityID entity) const override;
        std::string GetEntityIcon(EntityID entity) const override;
        bool HasChildren(EntityID entity) const override;
        EntityID GetParent(EntityID entity) const override;
        void SetParent(EntityID child, EntityID parent) override;
        EntityID CreateEntity(EntityTemplate templateType, EntityID parent = NULL_ENTITY) override;
        void DestroyEntityRecursive(EntityID entity) override;
        void DestroyEntityKeepChildren(EntityID entity) override;
        EntityID CopyEntity(EntityID entity, EntityID parent = NULL_ENTITY) override;
        EntityID FullCopyEntity(EntityID entity, EntityID parent = NULL_ENTITY) override;
        void DestroyEntity(EntityID entity) override;
    private:
        SceneManager* _sceneManager;
        ModelManager* _modelManager;
    };
}