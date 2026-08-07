#pragma once
#include "Engine/SynApi.h"
#include "IVideoLoaderRegistry.h"

namespace Syn
{
    class SYN_API VideoLoaderRegistry : public IVideoLoaderRegistry
    {
    public:
        void Register(std::shared_ptr<IVideoLoader> loader, int priority = 0) override;
        std::vector<IVideoLoader*> GetLoadersForExtension(const std::string& extension) override;
        IVideoLoader* GetLoaderForExtension(const std::string& extension) override;
    private:
        std::unordered_map<std::string, std::vector<VideoLoaderEntry>> _loaders;
    };
}