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