#pragma once
#include "EditorCore/Api/IEditorApi.h"
#include "EditorCore/Api/IRenderApi.h"
#include "EditorCore/Api/ITransformApi.h"
#include "Engine/Scene/SceneManager.h"
#include "Engine/Engine.h"
#include <unordered_map>
#include "Editor/Manager/GuiTextureManager.h"

namespace Syn {
    class EditorApiImpl : public IEditorApi {
    public:
        EditorApiImpl(Engine* engine, GuiTextureManager* textureManager) : _engine(engine), _sceneManager(engine->GetSceneManager()), _textureManager(textureManager) {}

        // --- ISelectionApi ---
        EntityID GetSelectedEntity() const override;
        void SetSelectedEntity(EntityID entity) override;

        // --- ITransformApi ---
        glm::vec3 GetEntityScale(EntityID entity) const override;
        glm::vec3 GetEntityPosition(EntityID entity) const override;
        glm::vec3 GetEntityRotation(EntityID entity) const override;
        void SetEntityScale(EntityID entity, const glm::vec3& scale) override;
        void SetEntityRotation(EntityID entity, const glm::vec3& rotation) override;
        void SetEntityPosition(EntityID entity, const glm::vec3& position) override;
        glm::mat4 GetEntityWorldMatrix(EntityID entity) const override;
        EntityID GetEntityParent(EntityID entity) const override;

        // --- IRenderApi ---
        void ResizeRenderTargets(uint32_t width, uint32_t height) override;
        TextureHandle GetViewportTexture(const std::string& groupName, const std::string& targetName, const std::string& viewName) override;
        EntityID ReadEntityIdAtPixel(uint32_t x, uint32_t y) override;
        glm::mat4 GetEditorCameraView() const override;
        glm::mat4 GetEditorCameraProjection() const override;

		// --- ISettingsApi ---
        SceneSettings GetSceneSettings() const override;
        void SetSceneSettings(const SceneSettings& settings) override;

        // --- ISceneApi ---
        void NewScene() override;
        void LoadScene(const std::string& filepath = "") override;
        void SaveScene(const std::string& filepath = "") override;

        // --- IMaterialApi ---
        std::vector<MaterialApiDesc> GetAllMaterials() const override;
        std::vector<TextureApiDesc> GetAllTextures() const override;
        void LinkTextureToMaterial(uint32_t materialId, uint32_t textureType, uint32_t textureId) override;
        void UnlinkTextureFromMaterial(uint32_t materialId, uint32_t textureType) override;

        // --- IFileSystemApi ---
        std::vector<FileEntry> GetEntries(const std::string& directoryPath) const override;
        std::string GetParentPath(const std::string& path) const override;
        bool IsValidPath(const std::string& path) const override;

        // --- IHierarchyApi ---
        std::vector<EntityID> GetRootEntities() const override;
        std::vector<EntityID> GetChildren(EntityID entity) const override;

        std::string GetEntityIcon(EntityID entity) const override;
        bool HasChildren(EntityID entity) const override;

        void SetParent(EntityID child, EntityID parent) override;

        EntityID CreateEntity(const std::string& name, EntityID parent = NULL_ENTITY) override;
        void DestroyEntity(EntityID entity) override;

        // --- ITagApi ---
        std::string GetEntityName(EntityID entity) const override;
        void SetEntityName(EntityID entity, const std::string& name) override;
        bool IsEntityEnabled(EntityID entity) const override;
        void SetEntityEnabled(EntityID entity, bool enabled) override;
        std::string GetEntityTag(EntityID entity) const override;
        void SetEntityTag(EntityID entity, const std::string& tag) override;

		uint64_t GetVersion() const override;

        // --- ILoggerApi ---
        const std::vector<LogMessage>& GetLogs() const override;
        void ClearLogs() override;
    private:
        Engine* _engine = nullptr;
        SceneManager* _sceneManager = nullptr;
        GuiTextureManager* _textureManager = nullptr;

        std::unordered_map<std::string, TextureHandle> _viewportTextures;
    };
}