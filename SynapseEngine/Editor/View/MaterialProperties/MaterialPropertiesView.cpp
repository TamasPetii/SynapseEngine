#include "MaterialPropertiesView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {
    void MaterialPropertiesView::Draw(MaterialPropertiesViewModel& vm) {
        const auto& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        if (ImGui::Begin(SYN_ICON_INFO_CIRCLE " Material Properties", nullptr, windowFlags)) {

            auto getCardState = [this](const char* name) -> bool& {
                std::string key(name);
                if (_cardStates.find(key) == _cardStates.end()) _cardStates[key] = true;
                return _cardStates[key];
                };

            if (!state.hasSelection) {
                ImGui::TextDisabled("No material selected.");
                ImGui::End();
                ImGui::PopStyleVar();
                return;
            }

            Material editedMat = state.materialData;
            bool isModified = false;

            constexpr const char* BasicPropsCard = "MatBasicPropsCard";
            if (Syn::UI::BeginCard(BasicPropsCard, SYN_ICON_SLIDERS_H, getCardState(BasicPropsCard))) {
                if (Syn::UI::BeginPropertyGrid("MatPropsGrid")) {

                    if (Syn::UI::PropertyColor4("Color", editedMat.color)) {
                        isModified = true;
                    }

                    if (Syn::UI::PropertyColor3("Emissive Color", editedMat.emissiveColor)) {
                        isModified = true;
                    }

                    if (Syn::UI::PropertyDragFloat("Emissive Intensity", editedMat.emissiveIntensity, 0.1f, 0.0f, 1000.0f, "%.2f")) {
                        isModified = true;
                    }

                    if (Syn::UI::PropertyDragFloat2("UV Scale", editedMat.uvScale, 0.05f, 0.0f, 0.0f, "%.2f")) {
                        isModified = true;
                    }

                    Syn::UI::PropertySeparator();

                    if (Syn::UI::PropertyDragFloat("Metalness", editedMat.metalness, 0.01f, 0.0f, 1.0f, "%.2f")) {
                        isModified = true;
                    }

                    if (Syn::UI::PropertyDragFloat("Roughness", editedMat.roughness, 0.01f, 0.0f, 1.0f, "%.2f")) {
                        isModified = true;
                    }

                    if (Syn::UI::PropertyDragFloat("AO Strength", editedMat.aoStrength, 0.01f, 0.0f, 1.0f, "%.2f")) {
                        isModified = true;
                    }

                    Syn::UI::PropertySeparator();

                    if (Syn::UI::PropertyCheckbox("Double Sided", editedMat.doubleSided)) {
                        isModified = true;
                    }

                    if (Syn::UI::PropertyCheckbox("Transparent", editedMat.isTransparent)) {
                        isModified = true;
                    }

                    Syn::UI::EndPropertyGrid();
                }
                Syn::UI::EndCard();
            }

            ImGui::Spacing();

            constexpr const char* TexturesCard = "MatTexturesCard";
            if (Syn::UI::BeginCard(TexturesCard, SYN_ICON_IMAGE, getCardState(TexturesCard))) {
                if (Syn::UI::BeginPropertyGrid("MatTexGrid")) {

                    DrawTextureSlot("Albedo", editedMat.albedoTexture, state.albedoName, state.availableTextures, isModified);
                    DrawTextureSlot("Normal", editedMat.normalTexture, state.normalName, state.availableTextures, isModified);
                    DrawTextureSlot("Metalness", editedMat.metalnessTexture, state.metalnessName, state.availableTextures, isModified);
                    DrawTextureSlot("Roughness", editedMat.roughnessTexture, state.roughnessName, state.availableTextures, isModified);
                    DrawTextureSlot("MetallicRoughness", editedMat.metallicRoughnessTexture, state.metallicRoughnessName, state.availableTextures, isModified);
                    DrawTextureSlot("Emissive", editedMat.emissiveTexture, state.emissiveName, state.availableTextures, isModified);
                    DrawTextureSlot("Ambient Occlusion", editedMat.ambientOcclusionTexture, state.aoName, state.availableTextures, isModified);

                    Syn::UI::EndPropertyGrid();
                }
                Syn::UI::EndCard();
            }

            if (isModified) {
                vm.Dispatch(UpdateMaterialPropertyIntent{ editedMat });
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void MaterialPropertiesView::DrawTextureSlot(const char* label, uint32_t& currentTexId, const std::string& currentName, const std::vector<TextureOption>& options, bool& changed) {

        if (Syn::UI::BeginPropertyCombo(label, currentName.c_str())) {

            if (ImGui::Selectable("None", currentTexId == 0xFFFFFFFF)) {
                if (currentTexId != 0xFFFFFFFF) {
                    currentTexId = 0xFFFFFFFF;
                    changed = true;
                }
            }

            for (const auto& opt : options) {
                bool isSelected = (currentTexId == opt.id);
                if (ImGui::Selectable(opt.name.c_str(), isSelected)) {
                    if (currentTexId != opt.id) {
                        currentTexId = opt.id;
                        changed = true;
                    }
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }

            Syn::UI::EndPropertyCombo();
        }
    }
}