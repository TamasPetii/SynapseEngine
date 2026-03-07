#include "ImageLoaderRegistry.h"

namespace Syn
{
    void ImageLoaderRegistry::Register(std::shared_ptr<IImageLoader> loader, int priority)
    {
        for (const auto& ext : loader->GetSupportedExtensions())
        {
            _loaders[ext].push_back({ priority, loader });
            std::sort(_loaders[ext].begin(), _loaders[ext].end(),
                [](const ImageLoaderEntry& a, const ImageLoaderEntry& b) {
                    return a.priority > b.priority;
                });
        }
    }

    std::vector<IImageLoader*> ImageLoaderRegistry::GetLoadersForExtension(const std::string& extension)
    {
        std::vector<IImageLoader*> result;
        auto it = _loaders.find(extension);
        if (it != _loaders.end()) {
            for (const auto& entry : it->second) {
                result.push_back(entry.loader.get());
            }
        }
        return result;
    }

    IImageLoader* ImageLoaderRegistry::GetLoaderForExtension(const std::string& extension)
    {
        auto it = _loaders.find(extension);
        if (it != _loaders.end() && !it->second.empty()) {
            return it->second.front().loader.get();
        }
        return nullptr;
    }
}