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

#include "AudioListenerView.h"
#include "Editor/Manager/EditorIcons.h" 
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {

    void AudioListenerView::Draw(AudioListenerViewModel& vm) {
        AudioListenerState state = vm.GetState();

        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Audio Listener";

        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_AUDIO, _isCardOpen))
        {
            if (Syn::UI::BeginPropertyGrid("AudioListenerGrid"))
            {
                Syn::UI::BeginProperty("Active");
                bool activeState = state.active;
                if (ImGui::Checkbox("##Active", &activeState)) {
                    vm.Dispatch(SetAudioListenerActiveIntent{ activeState });
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}