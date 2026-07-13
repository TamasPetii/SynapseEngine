#include "SvgImageLoader.h"
#include "Engine/Logger/SynLog.h"

#define NANOSVG_IMPLEMENTATION
#include <nanosvg/nanosvg.h>
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvg/nanosvgrast.h>

namespace Syn
{
    std::optional<RawImage> SvgImageLoader::LoadFile(const std::filesystem::path& path)
    {
        NSVGimage* image = nsvgParseFromFile(path.string().c_str(), "px", 96.0f);

        if (!image) {
            Error("Failed to parse SVG file: {}", path.string());
            return std::nullopt;
        }

        return ProcessImage(image);
    }

    std::optional<RawImage> SvgImageLoader::LoadMemory(const std::vector<uint8_t>& data)
    {
        if (data.empty()) return std::nullopt;

        std::vector<char> mutableData(data.begin(), data.end());
        mutableData.push_back('\0');

        NSVGimage* image = nsvgParse(mutableData.data(), "px", 96.0f);

        if (!image) {
            Error("SvgImageLoader failed to parse SVG from memory");
            return std::nullopt;
        }

        return ProcessImage(image);
    }

    std::optional<RawImage> SvgImageLoader::ProcessImage(NSVGimage* image)
    {
        int width = static_cast<int>(image->width);
        int height = static_cast<int>(image->height);

        NSVGrasterizer* rast = nsvgCreateRasterizer();
        if (!rast) {
            Error("Could not create SVG rasterizer");
            nsvgDelete(image);
            return std::nullopt;
        }

        RawImage rawImage{};
        rawImage.width = static_cast<uint32_t>(width);
        rawImage.height = static_cast<uint32_t>(height);
        rawImage.depth = 1;
        rawImage.mipLevels = 1;
        rawImage.isCompressed = false;
        rawImage.format = VK_FORMAT_R8G8B8A8_UNORM;

        size_t imageSize = static_cast<size_t>(width * height * 4);
        rawImage.pixels.resize(imageSize);

        nsvgRasterize(rast, image, 0.0f, 0.0f, 1.0f, rawImage.pixels.data(), width, height, width * 4);
        nsvgDeleteRasterizer(rast);
        nsvgDelete(image);

        return rawImage;
    }

    std::vector<std::string> SvgImageLoader::GetSupportedExtensions() const
    {
        return { ".svg" };
    }
}