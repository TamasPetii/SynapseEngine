#include "VideoLoaderRegistry.h"
#include <algorithm>

namespace Syn
{
    void VideoLoaderRegistry::Register(std::shared_ptr<IVideoLoader> loader, int priority)
    {
        for (const auto& ext : loader->GetSupportedExtensions())
        {
            _loaders[ext].push_back({ priority, loader });
            std::sort(_loaders[ext].begin(), _loaders[ext].end(),
                [](const VideoLoaderEntry& a, const VideoLoaderEntry& b) {
                    return a.priority > b.priority;
                });
        }
    }

    std::vector<IVideoLoader*> VideoLoaderRegistry::GetLoadersForExtension(const std::string& extension)
    {
        std::vector<IVideoLoader*> result;
        auto it = _loaders.find(extension);
        if (it != _loaders.end()) {
            for (const auto& entry : it->second) {
                result.push_back(entry.loader.get());
            }
        }
        return result;
    }

    IVideoLoader* VideoLoaderRegistry::GetLoaderForExtension(const std::string& extension)
    {
        auto it = _loaders.find(extension);
        if (it != _loaders.end() && !it->second.empty()) {
            return it->second.front().loader.get();
        }
        return nullptr;
    }
}