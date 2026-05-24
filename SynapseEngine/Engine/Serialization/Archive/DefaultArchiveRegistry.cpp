#include "DefaultArchiveRegistry.h"
#include <algorithm>

namespace Syn
{
    void DefaultArchiveRegistry::RegisterOutput(const std::string& ext, OutputArchiveFactory factory, int priority) {
        _outputFactories[ext].push_back({ priority, factory });
        std::sort(_outputFactories[ext].begin(), _outputFactories[ext].end(), [](auto& a, auto& b) { return a.priority > b.priority; });
    }

    std::unique_ptr<IOutputArchive> DefaultArchiveRegistry::CreateOutput(const std::string& ext, IOutputStream& stream) {
        auto it = _outputFactories.find(ext);
        return (it != _outputFactories.end() && !it->second.empty()) ? it->second.front().factory(stream) : nullptr;
    }

    void DefaultArchiveRegistry::RegisterInput(const std::string& ext, InputArchiveFactory factory, int priority) {
        _inputFactories[ext].push_back({ priority, factory });
        std::sort(_inputFactories[ext].begin(), _inputFactories[ext].end(), [](auto& a, auto& b) { return a.priority > b.priority; });
    }

    std::unique_ptr<IInputArchive> DefaultArchiveRegistry::CreateInput(const std::string& ext, IInputStream& stream) {
        auto it = _inputFactories.find(ext);
        return (it != _inputFactories.end() && !it->second.empty()) ? it->second.front().factory(stream) : nullptr;
    }
}