#include "AnimationLoaderRegistry.h"

namespace Syn
{
    void AnimationLoaderRegistry::Register(std::shared_ptr<IAnimationLoader> loader, int priority)
    {
        for (const auto& ext : loader->GetSupportedExtensions())
        {
            _loaders[ext].push_back({ priority, loader });

            std::sort(_loaders[ext].begin(), _loaders[ext].end(),
                [](const AnimationLoaderEntry& a, const AnimationLoaderEntry& b) {
                    return a.priority > b.priority;
                });
        }
    }

    std::vector<IAnimationLoader*> AnimationLoaderRegistry::GetLoadersForExtension(const std::string& extension)
    {
        std::vector<IAnimationLoader*> result;

        auto it = _loaders.find(extension);
        if (it != _loaders.end())
        {
            for (const auto& entry : it->second)
            {
                result.push_back(entry.loader.get());
            }
        }

        return result;
    }

    IAnimationLoader* AnimationLoaderRegistry::GetLoaderForExtension(const std::string& extension)
    {
        auto it = _loaders.find(extension);

        if (it != _loaders.end() && !it->second.empty())
        {
            return it->second.front().loader.get();
        }

        return nullptr;
    }
}