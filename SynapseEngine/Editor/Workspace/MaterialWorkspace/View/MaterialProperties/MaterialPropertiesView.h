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
#include "Editor/Workspace/IView.h"
#include "EditorCore/ViewModels/MaterialWorkspace/MaterialProperties/MaterialPropertiesViewModel.h"
#include <unordered_map>
#include <string>

namespace Syn {
    class MaterialPropertiesView : public IView<MaterialPropertiesViewModel> {
    public:
        void Draw(MaterialPropertiesViewModel& vm) override;
    private:
        void DrawTextureSlot(const char* label,
            uint32_t& currentTexId, const std::string& currentTexName, const std::vector<TextureOption>& texOptions,
            uint32_t& currentSampId, const std::string& currentSampName, const std::vector<SamplerOption>& sampOptions,
            bool& changed);
    private:
        std::unordered_map<std::string, bool> _cardStates;
    };
}