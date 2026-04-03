#pragma once
#include "Engine/SynApi.h"
#include "IImageLoaderRegistry.h"

namespace Syn
{
    class SYN_API ImageLoaderRegistry : public IImageLoaderRegistry
    {
    public:
        void Register(std::shared_ptr<IImageLoader> loader, int priority = 0) override;
        std::vector<IImageLoader*> GetLoadersForExtension(const std::string& extension) override;
        IImageLoader* GetLoaderForExtension(const std::string& extension) override;
    private:
        std::unordered_map<std::string, std::vector<ImageLoaderEntry>> _loaders;
    };
}