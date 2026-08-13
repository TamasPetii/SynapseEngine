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

#include "DefaultAudioLoaderRegistry.h"
#include <algorithm>

namespace Syn
{
    void DefaultAudioLoaderRegistry::Register(std::shared_ptr<IAudioLoader> loader, int priority)
    {
        for (const auto& ext : loader->GetSupportedExtensions())
        {
            _loaders[ext].push_back({ priority, loader });
            std::sort(_loaders[ext].begin(), _loaders[ext].end(),
                [](const AudioLoaderEntry& a, const AudioLoaderEntry& b) {
                    return a.priority > b.priority;
                });
        }
    }

    std::vector<IAudioLoader*> DefaultAudioLoaderRegistry::GetLoadersForExtension(const std::string& extension)
    {
        std::vector<IAudioLoader*> result;
        auto it = _loaders.find(extension);
        if (it != _loaders.end()) {
            for (const auto& entry : it->second) {
                result.push_back(entry.loader.get());
            }
        }
        return result;
    }

    IAudioLoader* DefaultAudioLoaderRegistry::GetLoaderForExtension(const std::string& extension)
    {
        auto it = _loaders.find(extension);
        if (it != _loaders.end() && !it->second.empty()) {
            return it->second.front().loader.get();
        }
        return nullptr;
    }
}