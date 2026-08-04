#pragma once
#include "Engine/SynApi.h"
#include "IAudioLoader.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

namespace Syn
{
    struct SYN_API AudioLoaderEntry
    {
        int priority;
        std::shared_ptr<IAudioLoader> loader;
    };


    class SYN_API IAudioLoaderRegistry
    {
    public:
        virtual ~IAudioLoaderRegistry() = default;
        virtual void Register(std::shared_ptr<IAudioLoader> loader, int priority = 0) = 0;
        virtual IAudioLoader* GetLoaderForExtension(const std::string& extension) = 0;
        virtual std::vector<IAudioLoader*> GetLoadersForExtension(const std::string& extension) = 0;
    };
}