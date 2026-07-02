#include "ModelComponentView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {
    void ModelComponentView::Draw(ModelComponentViewModel& vm) {
        ModelComponentState state = vm.GetState();
        if (!state.hasComponent) return;

        constexpr const char* CardTitle = "Model Component";

        if (Syn::UI::BeginCard(CardTitle, SYN_ICON_MODEL, _isCardOpen)) {
            if (Syn::UI::BeginPropertyGrid("ModelGrid"))
            {
                std::string previewName = "None (UINT32_MAX)";
                if (state.modelIndex != UINT32_MAX) {
                    for (const auto& model : state.availableModels) {
                        if (model.first == state.modelIndex) {
                            previewName = model.second;
                            break;
                        }
                    }
                }

                if (Syn::UI::BeginPropertyCombo("Model", previewName.c_str())) {
                    for (const auto& model : state.availableModels) {
                        bool isSelected = (state.modelIndex == model.first);

                        if (ImGui::Selectable(model.second.c_str(), isSelected)) {
                            vm.Dispatch(SetModelIndexIntent{ model.first });
                        }

                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    Syn::UI::EndPropertyCombo();
                }

                Syn::UI::PropertySeparator();

                bool castShadow = state.castShadow;
                if (Syn::UI::PropertyCheckbox("Cast Shadow", castShadow)) {
                    vm.Dispatch(SetModelCastShadowIntent{ castShadow });
                }

                bool receiveShadow = state.receiveShadow;
                if (Syn::UI::PropertyCheckbox("Receive Shadow", receiveShadow)) {
                    vm.Dispatch(SetModelReceiveShadowIntent{ receiveShadow });
                }

                Syn::UI::EndPropertyGrid();
            }
        }
        Syn::UI::EndCard();
    }
}