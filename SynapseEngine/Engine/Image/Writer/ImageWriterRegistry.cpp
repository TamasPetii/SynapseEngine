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

#include "ImageWriterRegistry.h"
#include <algorithm>

namespace Syn
{
    void ImageWriterRegistry::Register(std::shared_ptr<IImageWriter> writer, int priority)
    {
        for (const auto& ext : writer->GetSupportedExtensions())
        {
            _writers[ext].push_back({ priority, writer });
            std::sort(_writers[ext].begin(), _writers[ext].end(),
                [](const ImageWriterEntry& a, const ImageWriterEntry& b) {
                    return a.priority > b.priority;
                });
        }
    }

    std::vector<IImageWriter*> ImageWriterRegistry::GetWritersForExtension(const std::string& extension)
    {
        std::vector<IImageWriter*> result;
        auto it = _writers.find(extension);
        if (it != _writers.end()) {
            for (const auto& entry : it->second) {
                result.push_back(entry.writer.get());
            }
        }
        return result;
    }

    IImageWriter* ImageWriterRegistry::GetWriterForExtension(const std::string& extension)
    {
        auto it = _writers.find(extension);
        if (it != _writers.end() && !it->second.empty()) {
            return it->second.front().writer.get();
        }
        return nullptr;
    }
}