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

#include "AudioSourceView.h"
#include "Editor/Manager/EditorIcons.h" 
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {

    void AudioSourceView::Draw(AudioSourceViewModel& vm) {
        AudioSourceState state = vm.GetState();

        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Audio Source";

        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_AUDIO, _isCardOpen))
        {
            if (Syn::UI::BeginPropertyGrid("AudioSourceGrid"))
            {
                Syn::UI::BeginProperty("Sound Index");
                int soundIndexInt = static_cast<int>(state.soundIndex);
                if (ImGui::DragInt("##SoundIndex", &soundIndexInt, 1.0f, 0, 10000)) {
                    vm.Dispatch(SetAudioSourceSoundIndexIntent{ static_cast<uint32_t>(soundIndexInt) });
                }

                Syn::UI::PropertySeparator();

                Syn::UI::BeginProperty("Play");
                bool playState = state.play;
                if (ImGui::Checkbox("##Play", &playState)) {
                    vm.Dispatch(SetAudioSourcePlayIntent{ playState });
                }

                Syn::UI::BeginProperty("Loop");
                bool loopState = state.loop;
                if (ImGui::Checkbox("##Loop", &loopState)) {
                    vm.Dispatch(SetAudioSourceLoopIntent{ loopState });
                }

                Syn::UI::BeginProperty("Spatialized");
                bool spatializedState = state.isSpatialized;
                if (ImGui::Checkbox("##Spatialized", &spatializedState)) {
                    vm.Dispatch(SetAudioSourceIsSpatializedIntent{ spatializedState });
                }

                Syn::UI::PropertySeparator();

                if (Syn::UI::PropertyDragFloat("Volume", state.volume, 0.01f, 0.0f, 1.0f, "%.2f")) {
                    vm.Dispatch(SetAudioSourceVolumeIntent{ state.volume, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Pitch", state.pitch, 0.01f, 0.1f, 3.0f, "%.2f")) {
                    vm.Dispatch(SetAudioSourcePitchIntent{ state.pitch, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Min Distance", state.minDistance, 0.1f, 0.0f, 10000.0f, "%.2f")) {
                    vm.Dispatch(SetAudioSourceMinDistanceIntent{ state.minDistance, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                if (Syn::UI::PropertyDragFloat("Max Distance", state.maxDistance, 0.1f, 0.0f, 10000.0f, "%.2f")) {
                    vm.Dispatch(SetAudioSourceMaxDistanceIntent{ state.maxDistance, !ImGui::IsItemDeactivatedAfterEdit() });
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}