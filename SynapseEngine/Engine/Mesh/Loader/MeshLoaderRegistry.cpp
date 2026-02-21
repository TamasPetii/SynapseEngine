#include "MeshLoaderRegistry.h"

namespace Syn
{
    void MeshLoaderRegistry::Register(std::shared_ptr<IMeshLoader> loader, int priority)
    {
        for (const auto& ext : loader->GetSupportedExtensions())
        {
            _loaders[ext].push_back({ priority, loader });

            std::sort(_loaders[ext].begin(), _loaders[ext].end(),
                [](const LoaderEntry& a, const LoaderEntry& b) {
                    return a.priority > b.priority;
                });
        }
    }

    std::vector<IMeshLoader*> MeshLoaderRegistry::GetLoadersForExtension(const std::string& extension)
    {
        std::vector<IMeshLoader*> result;

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

    IMeshLoader* MeshLoaderRegistry::GetLoaderForExtension(const std::string& extension)
    {
        auto it = _loaders.find(extension);

        if (it != _loaders.end() && !it->second.empty())
        {
            return it->second.front().loader.get();
        }

        return nullptr;
    }
}