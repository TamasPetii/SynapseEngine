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