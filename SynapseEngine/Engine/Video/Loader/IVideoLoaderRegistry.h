#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Loader/IVideoLoader.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

namespace Syn
{
    struct SYN_API VideoLoaderEntry
    {
        int priority;
        std::shared_ptr<IVideoLoader> loader;
    };

    class SYN_API IVideoLoaderRegistry
    {
    public:
        virtual ~IVideoLoaderRegistry() = default;
        virtual void Register(std::shared_ptr<IVideoLoader> loader, int priority = 0) = 0;
        virtual IVideoLoader* GetLoaderForExtension(const std::string& extension) = 0;
        virtual std::vector<IVideoLoader*> GetLoadersForExtension(const std::string& extension) = 0;
    };
}