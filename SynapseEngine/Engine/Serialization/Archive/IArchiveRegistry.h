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
#include "Engine/Serialization/Archive/Output/IOutputArchive.h"
#include "Engine/Serialization/Archive/Input/IInputArchive.h"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>

namespace Syn
{
    using OutputArchiveFactory = std::function<std::unique_ptr<IOutputArchive>(IOutputStream&)>;
    using InputArchiveFactory = std::function<std::unique_ptr<IInputArchive>(IInputStream&)>;

    struct SYN_API OutputEntry { 
        int priority;
        OutputArchiveFactory factory;
    };

    struct SYN_API InputEntry {
        int priority;
        InputArchiveFactory factory;
    };

    class SYN_API IArchiveRegistry {
    public:
        virtual ~IArchiveRegistry() = default;

        virtual void RegisterOutput(const std::string& ext, OutputArchiveFactory factory, int priority = 0) = 0;
        virtual std::unique_ptr<IOutputArchive> CreateOutput(const std::string& ext, IOutputStream& stream) = 0;

        virtual void RegisterInput(const std::string& ext, InputArchiveFactory factory, int priority = 0) = 0;
        virtual std::unique_ptr<IInputArchive> CreateInput(const std::string& ext, IInputStream& stream) = 0;

        template <typename ArchiveType>
			requires std::derived_from<ArchiveType, IOutputArchive>
        void RegisterOutputAuto(int priority = 0) {
            for (const auto& ext : ArchiveType::GetSupportedExtensions()) {
                RegisterOutput(ext, [](IOutputStream& stream) {
                    return std::make_unique<ArchiveType>(stream);
                    }, priority);
            }
        }

        template <typename ArchiveType>
            requires std::derived_from<ArchiveType, IInputArchive>
        void RegisterInputAuto(int priority = 0) {
            for (const auto& ext : ArchiveType::GetSupportedExtensions()) {
                RegisterInput(ext, [](IInputStream& stream) {
                    return std::make_unique<ArchiveType>(stream);
                    }, priority);
            }
        }
    };
}