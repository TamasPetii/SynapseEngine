#pragma once
#include "Engine/SynApi.h"
#include "IAudioLoaderRegistry.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

namespace Syn
{
    class SYN_API DefaultAudioLoaderRegistry : public IAudioLoaderRegistry
    {
    public:
        void Register(std::shared_ptr<IAudioLoader> loader, int priority = 0) override;
        std::vector<IAudioLoader*> GetLoadersForExtension(const std::string& extension) override;
        IAudioLoader* GetLoaderForExtension(const std::string& extension) override;
    private:
        std::unordered_map<std::string, std::vector<AudioLoaderEntry>> _loaders;
    };
}