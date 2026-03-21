#pragma once
#include "Engine/SynApi.h"
#include "IMeshLoaderRegistry.h"

namespace Syn
{
    class SYN_API MeshLoaderRegistry : public IMeshLoaderRegistry
    {
    public:
        virtual void Register(std::shared_ptr<IMeshLoader> loader, int priority = 0) override;
        virtual std::vector<IMeshLoader*> GetLoadersForExtension(const std::string& extension) override;
        virtual IMeshLoader* GetLoaderForExtension(const std::string& extension) override;
    private:
        std::unordered_map<std::string, std::vector<MeshLoaderEntry>> _loaders;
    };
}