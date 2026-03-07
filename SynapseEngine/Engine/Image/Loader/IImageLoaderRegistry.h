#pragma once
#include "Engine/SynApi.h"
#include "IImageLoader.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

namespace Syn
{
    struct SYN_API ImageLoaderEntry
    {
        int priority;
        std::shared_ptr<IImageLoader> loader;
    };

    class SYN_API IImageLoaderRegistry
    {
    public:
        virtual ~IImageLoaderRegistry() = default;
        virtual void Register(std::shared_ptr<IImageLoader> loader, int priority = 0) = 0;
        virtual IImageLoader* GetLoaderForExtension(const std::string& extension) = 0;
        virtual std::vector<IImageLoader*> GetLoadersForExtension(const std::string& extension) = 0;
    };
}