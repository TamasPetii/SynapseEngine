#include "DefaultAudioLoaderRegistry.h"
#include <algorithm>

namespace Syn
{
    void DefaultAudioLoaderRegistry::Register(std::shared_ptr<IAudioLoader> loader, int priority)
    {
        for (const auto& ext : loader->GetSupportedExtensions())
        {
            _loaders[ext].push_back({ priority, loader });
            std::sort(_loaders[ext].begin(), _loaders[ext].end(),
                [](const AudioLoaderEntry& a, const AudioLoaderEntry& b) {
                    return a.priority > b.priority;
                });
        }
    }

    std::vector<IAudioLoader*> DefaultAudioLoaderRegistry::GetLoadersForExtension(const std::string& extension)
    {
        std::vector<IAudioLoader*> result;
        auto it = _loaders.find(extension);
        if (it != _loaders.end()) {
            for (const auto& entry : it->second) {
                result.push_back(entry.loader.get());
            }
        }
        return result;
    }

    IAudioLoader* DefaultAudioLoaderRegistry::GetLoaderForExtension(const std::string& extension)
    {
        auto it = _loaders.find(extension);
        if (it != _loaders.end() && !it->second.empty()) {
            return it->second.front().loader.get();
        }
        return nullptr;
    }
}