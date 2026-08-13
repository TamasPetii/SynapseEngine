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
#include "IImageLoader.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

namespace Syn
{
    struct SYN_API ImageLoaderEntry
    {
        int priority;
        std::shared_ptr<IImageLoader> loader;
    };

    class SYN_API IImageLoaderRegistry
    {
    public:
        virtual ~IImageLoaderRegistry() = default;
        virtual void Register(std::shared_ptr<IImageLoader> loader, int priority = 0) = 0;
        virtual IImageLoader* GetLoaderForExtension(const std::string& extension) = 0;
        virtual std::vector<IImageLoader*> GetLoadersForExtension(const std::string& extension) = 0;
    };
}