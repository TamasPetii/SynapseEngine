#include "TransformView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/Vector3Widget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {

    void TransformView::Draw(TransformViewModel& vm) {
        constexpr const char* CardTransformTitle = "Transform";

        if (Syn::UI::BeginCard(CardTransformTitle, SYN_ICON_ARROWS_ALT, _isCardOpen)) {

            TransformState tState = vm.GetState();
            bool changed = false;
            bool deactivated = false;

            if (Syn::UI::BeginPropertyGrid("TransformGrid")) {

                Syn::UI::BeginProperty("Position");
                changed = Syn::UI::DrawVec3Control("##Pos", tState.position, 0.0f, deactivated);
                if (changed || deactivated) {
                    vm.Dispatch(SetPositionIntent{ tState.position, !deactivated });
                }

                Syn::UI::BeginProperty("Rotation");
                changed = Syn::UI::DrawVec3Control("##Rot", tState.rotation, 0.0f, deactivated);
                if (changed || deactivated) {
                    vm.Dispatch(SetRotationIntent{ tState.rotation, !deactivated });
                }

                Syn::UI::BeginProperty("Scale");
                changed = Syn::UI::DrawVec3Control("##Scale", tState.scale, 1.0f, deactivated);
                if (changed || deactivated) {
                    vm.Dispatch(SetScaleIntent{ tState.scale, !deactivated });
                }

                Syn::UI::EndPropertyGrid();
            }
        }

        Syn::UI::EndCard();
    }
}