#pragma once
#include "EditorCore/Api/IEditorAPI.h"
#include "EditorCore/Api/IRenderAPI.h"
#include "EditorCore/Api/ITransformAPI.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Engine.h"
#include <unordered_map>

namespace Syn {
    class EditorApiImpl : public IEditorAPI {
    public:
        EditorApiImpl(Engine* engine) : _engine(engine), _sceneManager(engine->GetSceneManager()) {}

        // --- ISelectionAPI ---
        EntityID GetSelectedEntity() const override;
        void SetSelectedEntity(EntityID entity) override;

        // --- ITransformAPI ---
        glm::vec3 GetEntityScale(EntityID entity) const override;
        glm::vec3 GetEntityPosition(EntityID entity) const override;
        glm::vec3 GetEntityRotation(EntityID entity) const override;
        void SetEntityScale(EntityID entity, const glm::vec3& scale) override;
        void SetEntityRotation(EntityID entity, const glm::vec3& rotation) override;
        void SetEntityPosition(EntityID entity, const glm::vec3& position) override;
        glm::mat4 GetEntityWorldMatrix(EntityID entity) const override;
        EntityID GetEntityParent(EntityID entity) const override;

        // --- IRenderAPI ---
        void ResizeRenderTargets(uint32_t width, uint32_t height) override;
        TextureHandle GetViewportTexture(const std::string& groupName, const std::string& targetName, const std::string& viewName) override;
        EntityID ReadEntityIdAtPixel(uint32_t x, uint32_t y) override;
        glm::mat4 GetEditorCameraView() const override;
        glm::mat4 GetEditorCameraProjection() const override;
    private:
        Engine* _engine = nullptr;
        SceneManager* _sceneManager = nullptr;
        EntityID _selectedEntity = NULL_ENTITY;

        std::unordered_map<std::string, TextureHandle> _viewportTextures;
    };
}