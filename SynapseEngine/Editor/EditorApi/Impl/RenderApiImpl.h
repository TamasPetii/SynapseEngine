#pragma once
#include "EditorCore/Api/IRenderApi.h"
#include "Engine/Engine.h"
#include "Engine/Scene/SceneManager.h"
#include "Editor/Manager/GuiTextureManager.h"
#include <unordered_map>

namespace Syn {
    class RenderApiImpl : public IRenderApi {
    public:
        RenderApiImpl(Engine* engine, GuiTextureManager* textureManager, SceneManager* sm) 
            : _engine(engine), _textureManager(textureManager), _sceneManager(sm) {}

        void ResizeRenderTargets(uint32_t width, uint32_t height) override;
        TextureHandle GetViewportTexture(const std::string& groupName, const std::string& targetName, const std::string& viewName) override;
        EntityID ReadEntityIdAtPixel(uint32_t x, uint32_t y) override;
        std::pair<EntityID, uint32_t> ReadEntityAndMeshIdAtPixel(uint32_t x, uint32_t y) override;
        glm::mat4 GetEditorCameraView() const override;
        glm::mat4 GetEditorCameraProjection() const override;
    private:
        Engine* _engine;
        GuiTextureManager* _textureManager;
        SceneManager* _sceneManager;
        std::unordered_map<std::string, TextureHandle> _viewportTextures;
    };
}