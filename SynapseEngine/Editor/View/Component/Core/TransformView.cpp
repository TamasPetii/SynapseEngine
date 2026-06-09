#include "TransformView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/Vector3Widget.h"
#include <imgui.h>

namespace Syn {

    void TransformView::Draw(TransformViewModel& vm) {
        constexpr const char* CardTransformTitle = "Transform";

        if (Syn::UI::BeginCard(CardTransformTitle, SYN_ICON_ARROWS_ALT, _isCardOpen)) {

            TransformState tState = vm.GetState();
            bool changed = false;
            bool deactivated = false;

            changed = Syn::UI::DrawVec3Control("Position", tState.position, 0.0f, deactivated);
            if (changed || deactivated) {
                vm.Dispatch(SetPositionIntent{ tState.position, !deactivated });
            }

            changed = Syn::UI::DrawVec3Control("Rotation", tState.rotation, 0.0f, deactivated);
            if (changed || deactivated) {
                vm.Dispatch(SetRotationIntent{ tState.rotation, !deactivated });
            }

            changed = Syn::UI::DrawVec3Control("Scale", tState.scale, 1.0f, deactivated);
            if (changed || deactivated) {
                vm.Dispatch(SetScaleIntent{ tState.scale, !deactivated });
            }
        }
        Syn::UI::EndCard();
    }
}