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
#include "IImageLoaderRegistry.h"

namespace Syn
{
    class SYN_API ImageLoaderRegistry : public IImageLoaderRegistry
    {
    public:
        void Register(std::shared_ptr<IImageLoader> loader, int priority = 0) override;
        std::vector<IImageLoader*> GetLoadersForExtension(const std::string& extension) override;
        IImageLoader* GetLoaderForExtension(const std::string& extension) override;
    private:
        std::unordered_map<std::string, std::vector<ImageLoaderEntry>> _loaders;
    };
}