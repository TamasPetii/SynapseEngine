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