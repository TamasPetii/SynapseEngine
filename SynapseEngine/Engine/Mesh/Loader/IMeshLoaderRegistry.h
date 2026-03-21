#pragma once
#include "Engine/SynApi.h"
#include "IMeshLoader.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

namespace Syn
{
    struct SYN_API MeshLoaderEntry
    {
        int priority;
        std::shared_ptr<IMeshLoader> loader;
    };

    class SYN_API IMeshLoaderRegistry
    {
    public:
        virtual void Register(std::shared_ptr<IMeshLoader> loader, int priority = 0) = 0;
        virtual IMeshLoader* GetLoaderForExtension(const std::string& extension) = 0;
        virtual std::vector<IMeshLoader*> GetLoadersForExtension(const std::string& extension) = 0;
    };
}