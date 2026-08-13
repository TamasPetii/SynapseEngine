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
#include "IApi.h"
#include <string>
#include <functional>

namespace Syn {

    struct FileDialogArgs {
        std::string Title;
        std::string Filters;
        std::string DefaultPath;
    };

    class IFileDialogApi : public IApi {
    public:
        virtual ~IFileDialogApi() = default;

        virtual void OpenFile(const FileDialogArgs& args, std::function<void(const std::string&)> onResult) = 0;
        virtual void SaveFile(const FileDialogArgs& args, std::function<void(const std::string&)> onResult) = 0;
        virtual void Draw() = 0;
    };
}