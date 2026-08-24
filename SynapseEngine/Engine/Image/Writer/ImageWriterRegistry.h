#pragma once
#include "Engine/SynApi.h"
#include "IImageWriterRegistry.h"
#include <unordered_map>

namespace Syn
{
    class SYN_API ImageWriterRegistry : public IImageWriterRegistry
    {
    public:
        void Register(std::shared_ptr<IImageWriter> writer, int priority = 0) override;
        std::vector<IImageWriter*> GetWritersForExtension(const std::string& extension) override;
        IImageWriter* GetWriterForExtension(const std::string& extension) override;
    private:
        std::unordered_map<std::string, std::vector<ImageWriterEntry>> _writers;
    };
}