#include "TextureApiImpl.h"
#include "Engine/Image/SamplerNames.h"
#include <filesystem>

namespace Syn {

    std::vector<TextureItemData> TextureApiImpl::GetAllTextures() const {
        if (!_imageManager) return {};

        std::vector<TextureItemData> result;
        auto paths = _imageManager->GetResourcePaths();

        for (uint32_t i = 0; i < paths.size(); ++i) {
            if (_imageManager->GetEntryState(i) == ResourceState::Ready) {
                TextureItemData data;
                data.id = i;
                data.path = paths[i];

                std::filesystem::path p(paths[i]);
                data.name = p.filename().string();

                result.push_back(data);
            }
        }

        return result;
    }

    uint32_t TextureApiImpl::GetSelectedTexture() const {
        return _selectedTexture;
    }

    void TextureApiImpl::SetSelectedTexture(uint32_t id) {
        _selectedTexture = id;
    }

    bool TextureApiImpl::GetTextureData(uint32_t id, CpuTextureData& outData) const {
        if (!_imageManager || id == INVALID_TEXTURE_ID) return false;

        auto resource = _imageManager->GetResource(id);
        if (resource) {
            outData = resource->cpuData;
            return true;
        }

        return false;
    }

    uint64_t TextureApiImpl::GetVersion() const {
        return _imageManager ? _imageManager->GetVersion() : 0;
    }

    TextureHandle TextureApiImpl::GetTextureHandle(uint32_t id) {
        if (!_imageManager || id == INVALID_TEXTURE_ID) {
            return InvalidTextureHandle;
        }

        if (_textureHandleCache.find(id) == _textureHandleCache.end()) {
            auto resource = _imageManager->GetResource(id);
            if (!resource || !resource->image) {
                return InvalidTextureHandle;
            }

            auto sampler = _imageManager->GetSampler(SamplerNames::LinearClampEdge);
            TextureHandle handle = _guiTextureManager->RegisterTexture(
                resource->image->GetView(),
                sampler->Handle()
            );

            _textureHandleCache[id] = handle;
        }

        return _guiTextureManager->GetImGuiTextureID(_textureHandleCache[id]);
    }
}