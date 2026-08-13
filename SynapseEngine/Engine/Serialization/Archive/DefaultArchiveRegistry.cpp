// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

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