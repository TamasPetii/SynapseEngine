#pragma once
#include "EditorCore/Api/ITextureApi.h"
#include "Engine/Image/ImageManager.h"
#include "Editor/Manager/GuiTextureManager.h"

namespace Syn {
    class TextureApiImpl : public ITextureApi {
    public:
        TextureApiImpl(ImageManager* imageManager, GuiTextureManager* guiTextureManager) 
            : _imageManager(imageManager), _guiTextureManager(guiTextureManager) {}

        std::vector<TextureItemData> GetAllTextures() override;
        std::vector<SamplerItemData> GetAllSamplers() const override;

        uint32_t GetSelectedTexture() const override;
        void SetSelectedTexture(uint32_t id) override;

        bool GetTextureData(uint32_t id, CpuTextureData& outData) const override;
        uint64_t GetVersion() const override;
        TextureHandle GetTextureHandle(uint32_t id) override;
    private:
        ImageManager* _imageManager;
        GuiTextureManager* _guiTextureManager;
        uint32_t _selectedTexture = INVALID_TEXTURE_ID;
        std::unordered_map<uint32_t, TextureHandle> _textureHandleCache;
    };
}