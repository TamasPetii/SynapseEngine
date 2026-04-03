#pragma once
#include "Engine/SynApi.h"
#include "IAnimationLoader.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

namespace Syn
{
    struct SYN_API AnimationLoaderEntry
    {
        int priority;
        std::shared_ptr<IAnimationLoader> loader;
    };

    class SYN_API IAnimationLoaderRegistry
    {
    public:
        virtual void Register(std::shared_ptr<IAnimationLoader> loader, int priority = 0) = 0;
        virtual IAnimationLoader* GetLoaderForExtension(const std::string& extension) = 0;
        virtual std::vector<IAnimationLoader*> GetLoadersForExtension(const std::string& extension) = 0;
    };
}