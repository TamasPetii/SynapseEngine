#pragma once
#include "Engine/SynApi.h"
#include "IImageWriter.h"
#include <memory>
#include <string>
#include <vector>

namespace Syn
{
    struct SYN_API ImageWriterEntry
    {
        int priority;
        std::shared_ptr<IImageWriter> writer;
    };

    class SYN_API IImageWriterRegistry
    {
    public:
        virtual ~IImageWriterRegistry() = default;
        virtual void Register(std::shared_ptr<IImageWriter> writer, int priority = 0) = 0;
        virtual IImageWriter* GetWriterForExtension(const std::string& extension) = 0;
        virtual std::vector<IImageWriter*> GetWritersForExtension(const std::string& extension) = 0;
    };
}