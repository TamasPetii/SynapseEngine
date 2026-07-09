#pragma once
#include <string>
#include <vector>
#include "Engine/Image/Data/Cpu/CpuTextureData.h"
#include "EditorCore/Types/TextureHandle.h"

namespace Syn {
    constexpr uint32_t INVALID_TEXTURE_ID = 0xFFFFFFFF;
    constexpr uint32_t INVALID_SAMPLER_ID = 0xFFFFFFFF;

    struct TextureItemData {
        uint32_t id;
        std::string name;
        std::string path;
    };

    struct SamplerItemData {
        uint32_t id;
        std::string name;
    };

    class ITextureApi {
    public:
        virtual ~ITextureApi() = default;

        virtual std::vector<TextureItemData> GetAllTextures() const = 0;
        virtual std::vector<SamplerItemData> GetAllSamplers() const = 0;

        virtual uint32_t GetSelectedTexture() const = 0;
        virtual void SetSelectedTexture(uint32_t id) = 0;
        virtual bool GetTextureData(uint32_t id, CpuTextureData& outData) const = 0;
        virtual uint64_t GetVersion() const = 0;
        virtual TextureHandle GetTextureHandle(uint32_t id) = 0;
    };
}