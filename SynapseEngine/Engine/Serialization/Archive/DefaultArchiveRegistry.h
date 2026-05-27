#pragma once
#include "IArchiveRegistry.h"

namespace Syn
{
    class SYN_API DefaultArchiveRegistry : public IArchiveRegistry {
    public:
        void RegisterOutput(const std::string& ext, OutputArchiveFactory factory, int priority = 0) override;
        std::unique_ptr<IOutputArchive> CreateOutput(const std::string& ext, IOutputStream& stream) override;

        void RegisterInput(const std::string& ext, InputArchiveFactory factory, int priority = 0) override;
        std::unique_ptr<IInputArchive> CreateInput(const std::string& ext, IInputStream& stream) override;
    private:
        std::unordered_map<std::string, std::vector<OutputEntry>> _outputFactories;
        std::unordered_map<std::string, std::vector<InputEntry>> _inputFactories;
    };
}