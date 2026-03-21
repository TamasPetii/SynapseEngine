#pragma once
#include "Engine/SynApi.h"
#include "IAnimationLoaderRegistry.h"

namespace Syn
{
    class SYN_API AnimationLoaderRegistry : public IAnimationLoaderRegistry
    {
    public:
        virtual void Register(std::shared_ptr<IAnimationLoader> loader, int priority = 0) override;
        virtual std::vector<IAnimationLoader*> GetLoadersForExtension(const std::string& extension) override;
        virtual IAnimationLoader* GetLoaderForExtension(const std::string& extension) override;
    private:
        std::unordered_map<std::string, std::vector<AnimationLoaderEntry>> _loaders;
    };
}