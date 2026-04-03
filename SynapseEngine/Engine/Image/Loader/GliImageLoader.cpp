#include "GliImageLoader.h"
#include <gli/gli.hpp>
#include <iostream>
#include <unordered_map>
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    VkFormat GliImageLoader::GliFormatToVulkan(int format) const
    {
        static const std::unordered_map<gli::format, VkFormat> formatMap = {
            // BC (DXT) compressed formats
            {gli::FORMAT_RGB_DXT1_UNORM_BLOCK8, VK_FORMAT_BC1_RGB_UNORM_BLOCK},
            {gli::FORMAT_RGBA_DXT1_UNORM_BLOCK8, VK_FORMAT_BC1_RGBA_UNORM_BLOCK},
            {gli::FORMAT_RGBA_DXT3_UNORM_BLOCK16, VK_FORMAT_BC2_UNORM_BLOCK},
            {gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16, VK_FORMAT_BC3_UNORM_BLOCK},
            {gli::FORMAT_R_ATI1N_UNORM_BLOCK8, VK_FORMAT_BC4_UNORM_BLOCK},
            {gli::FORMAT_RG_ATI2N_UNORM_BLOCK16, VK_FORMAT_BC5_UNORM_BLOCK},
            {gli::FORMAT_RGBA_BP_UNORM_BLOCK16, VK_FORMAT_BC7_UNORM_BLOCK},

            // Uncompressed formats
            {gli::FORMAT_RGBA8_UNORM_PACK8, VK_FORMAT_R8G8B8A8_UNORM},
            {gli::FORMAT_RGB8_UNORM_PACK8, VK_FORMAT_R8G8B8_UNORM},
            {gli::FORMAT_R8_UNORM_PACK8, VK_FORMAT_R8_UNORM},
            {gli::FORMAT_RG8_UNORM_PACK8, VK_FORMAT_R8G8_UNORM},
            {gli::FORMAT_RGBA16_UNORM_PACK16, VK_FORMAT_R16G16B16A16_UNORM},
            {gli::FORMAT_RGB16_UNORM_PACK16, VK_FORMAT_R16G16B16_UNORM},
            {gli::FORMAT_R16_UNORM_PACK16, VK_FORMAT_R16_UNORM},

            // SRGB formats
            {gli::FORMAT_RGB_DXT1_SRGB_BLOCK8, VK_FORMAT_BC1_RGB_SRGB_BLOCK},
            {gli::FORMAT_RGBA_DXT1_SRGB_BLOCK8, VK_FORMAT_BC1_RGBA_SRGB_BLOCK},
            {gli::FORMAT_RGBA_DXT3_SRGB_BLOCK16, VK_FORMAT_BC2_SRGB_BLOCK},
            {gli::FORMAT_RGBA_DXT5_SRGB_BLOCK16, VK_FORMAT_BC3_SRGB_BLOCK},
            {gli::FORMAT_RGBA_BP_SRGB_BLOCK16, VK_FORMAT_BC7_SRGB_BLOCK},
            {gli::FORMAT_RGBA8_SRGB_PACK8, VK_FORMAT_R8G8B8A8_SRGB},
            {gli::FORMAT_RGB8_SRGB_PACK8, VK_FORMAT_R8G8B8_SRGB},

            // Floating-point formats
            {gli::FORMAT_RGBA16_SFLOAT_PACK16, VK_FORMAT_R16G16B16A16_SFLOAT},
            {gli::FORMAT_RGB16_SFLOAT_PACK16, VK_FORMAT_R16G16B16_SFLOAT},
            {gli::FORMAT_R16_SFLOAT_PACK16, VK_FORMAT_R16_SFLOAT},
            {gli::FORMAT_RGBA32_SFLOAT_PACK32, VK_FORMAT_R32G32B32A32_SFLOAT},
            {gli::FORMAT_RGB32_SFLOAT_PACK32, VK_FORMAT_R32G32B32_SFLOAT},
            {gli::FORMAT_R32_SFLOAT_PACK32, VK_FORMAT_R32_SFLOAT},
        };

        auto it = formatMap.find(static_cast<gli::format>(format));
        if (it != formatMap.end()) {
            return it->second;
        }
        return VK_FORMAT_UNDEFINED;
    }

    std::optional<RawImage> GliImageLoader::LoadFile(const std::filesystem::path& path)
    {
        gli::texture texture = gli::load(path.string());

        if (texture.empty()) {
			Error("Failed to load/parse DDS file: {}", path.string());
            return std::nullopt;
        }

        if (texture.target() != gli::target::TARGET_2D || texture.layers() != 1) {
			Error("Only 2D textures with 1 layer are supported right now: {}", path.string());
            return std::nullopt;
        }

        RawImage rawImage{};
        rawImage.width = texture.extent().x;
        rawImage.height = texture.extent().y;
        rawImage.depth = 1;
        rawImage.format = GliFormatToVulkan(texture.format());
        rawImage.mipLevels = texture.levels();
        rawImage.isCompressed = gli::is_compressed(texture.format());

        if (rawImage.format == VK_FORMAT_UNDEFINED) {
			Error("Unsupported GLI format for file: {}", path.string());
            return std::nullopt;
        }

        rawImage.pixels.resize(texture.size());
        std::memcpy(rawImage.pixels.data(), texture.data(), texture.size());

        uint32_t currentOffset = 0;
        for (uint32_t level = 0; level < texture.levels(); ++level)
        {
            MipLevelInfo mipInfo{};
            mipInfo.width = texture.extent(level).x;
            mipInfo.height = texture.extent(level).y;
            mipInfo.size = texture.size(level);
            mipInfo.offset = currentOffset;

            rawImage.mipData.push_back(mipInfo);
            currentOffset += mipInfo.size;
        }

        return rawImage;
    }

    std::vector<std::string> GliImageLoader::GetSupportedExtensions() const
    {
        return { ".dds", ".ktx" };
    }
}