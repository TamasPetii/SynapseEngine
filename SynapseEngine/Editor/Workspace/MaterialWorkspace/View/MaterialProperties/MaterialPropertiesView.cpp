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

#include "MaterialPropertiesView.h"
#include "Editor/Manager/EditorIcons.h"
#include "Editor/Widgets/CardWidget.h"
#include "Editor/Widgets/PropertyGrid.h"
#include <imgui.h>

namespace Syn {
    void MaterialPropertiesView::Draw(MaterialPropertiesViewModel& vm) {
        const auto& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;

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
                    if (Syn::UI::PropertyColor4("Color", editedMat.color)) isModified = true;
                    if (Syn::UI::PropertyColor3("Emissive Color", editedMat.emissiveColor)) isModified = true;
                    if (Syn::UI::PropertyDragFloat("Emissive Intensity", editedMat.emissiveIntensity, 0.1f, 0.0f, 1000.0f, "%.2f")) isModified = true;
                    if (Syn::UI::PropertyDragFloat2("UV Scale", editedMat.uvScale, 0.05f, 0.0f, 0.0f, "%.2f")) isModified = true;

                    Syn::UI::PropertySeparator();

                    if (Syn::UI::PropertyDragFloat("Metalness", editedMat.metalness, 0.01f, 0.0f, 1.0f, "%.2f")) isModified = true;
                    if (Syn::UI::PropertyDragFloat("Roughness", editedMat.roughness, 0.01f, 0.0f, 1.0f, "%.2f")) isModified = true;
                    if (Syn::UI::PropertyDragFloat("AO Strength", editedMat.aoStrength, 0.01f, 0.0f, 1.0f, "%.2f")) isModified = true;

                    Syn::UI::PropertySeparator();

                    if (Syn::UI::PropertyDragFloat("Clearcoat", editedMat.clearcoatFactor, 0.01f, 0.0f, 1.0f, "%.2f")) isModified = true;
                    if (Syn::UI::PropertyDragFloat("CC Roughness", editedMat.clearcoatRoughness, 0.01f, 0.0f, 1.0f, "%.2f")) isModified = true;

                    if (Syn::UI::PropertyDragFloat("IOR", editedMat.ior, 0.01f, 1.0f, 3.0f, "%.2f")) isModified = true;
                    if (Syn::UI::PropertyDragFloat("Specular Factor", editedMat.specularFactor, 0.01f, 0.0f, 1.0f, "%.2f")) isModified = true;
                    if (Syn::UI::PropertyColor3("Specular Color", editedMat.specularColor)) isModified = true;

                    Syn::UI::PropertySeparator();

                    if (Syn::UI::PropertyCheckbox("Double Sided", editedMat.doubleSided)) isModified = true;
                    if (Syn::UI::PropertyCheckbox("Transparent", editedMat.isTransparent)) isModified = true;
                    if (Syn::UI::PropertyCheckbox("Alpha Tested", editedMat.isAlphaTested)) isModified = true;

                    Syn::UI::EndPropertyGrid();
                }
            }
            Syn::UI::EndCard();

            ImGui::Spacing();

            constexpr const char* TexturesCard = "MatTexturesCard";
            if (Syn::UI::BeginCard(TexturesCard, SYN_ICON_IMAGE, getCardState(TexturesCard))) {
                if (Syn::UI::BeginPropertyGrid("MatTexGrid")) {

                    DrawTextureSlot("Albedo",
                        editedMat.albedoTexture, state.albedoName, state.availableTextures,
                        editedMat.albedoSampler, state.albedoSamplerName, state.availableSamplers, isModified);

                    DrawTextureSlot("Normal",
                        editedMat.normalTexture, state.normalName, state.availableTextures,
                        editedMat.normalSampler, state.normalSamplerName, state.availableSamplers, isModified);

                    DrawTextureSlot("Metalness",
                        editedMat.metalnessTexture, state.metalnessName, state.availableTextures,
                        editedMat.metalnessSampler, state.metalnessSamplerName, state.availableSamplers, isModified);

                    DrawTextureSlot("Roughness",
                        editedMat.roughnessTexture, state.roughnessName, state.availableTextures,
                        editedMat.roughnessSampler, state.roughnessSamplerName, state.availableSamplers, isModified);

                    DrawTextureSlot("MetallicRoughness",
                        editedMat.metallicRoughnessTexture, state.metallicRoughnessName, state.availableTextures,
                        editedMat.metallicRoughnessSampler, state.metallicRoughnessSamplerName, state.availableSamplers, isModified);

                    DrawTextureSlot("Emissive",
                        editedMat.emissiveTexture, state.emissiveName, state.availableTextures,
                        editedMat.emissiveSampler, state.emissiveSamplerName, state.availableSamplers, isModified);

                    DrawTextureSlot("AmbientOcclusion",
                        editedMat.ambientOcclusionTexture, state.aoName, state.availableTextures,
                        editedMat.ambientOcclusionSampler, state.aoSamplerName, state.availableSamplers, isModified);

                    DrawTextureSlot("Opacity",
                        editedMat.opacityTexture, state.opacityName, state.availableTextures,
                        editedMat.opacitySampler, state.opacitySamplerName, state.availableSamplers, isModified);

                    DrawTextureSlot("Clearcoat",
                        editedMat.clearcoatTexture, state.clearcoatName, state.availableTextures,
                        editedMat.clearcoatSampler, state.clearcoatSamplerName, state.availableSamplers, isModified);

                    DrawTextureSlot("CC Roughness",
                        editedMat.clearcoatRoughnessTexture, state.clearcoatRoughnessName, state.availableTextures,
                        editedMat.clearcoatRoughnessSampler, state.clearcoatRoughnessSamplerName, state.availableSamplers, isModified);

                    DrawTextureSlot("CC Normal",
                        editedMat.clearcoatNormalTexture, state.clearcoatNormalName, state.availableTextures,
                        editedMat.clearcoatNormalSampler, state.clearcoatNormalSamplerName, state.availableSamplers, isModified);

                    DrawTextureSlot("Specular",
                        editedMat.specularTexture, state.specularName, state.availableTextures,
                        editedMat.specularSampler, state.specularSamplerName, state.availableSamplers, isModified);

                    DrawTextureSlot("Specular Color",
                        editedMat.specularColorTexture, state.specularColorName, state.availableTextures,
                        editedMat.specularColorSampler, state.specularColorSamplerName, state.availableSamplers, isModified);

                    Syn::UI::EndPropertyGrid();
                }
            }
            Syn::UI::EndCard();

            if (isModified) {
                vm.Dispatch(UpdateMaterialPropertyIntent{ editedMat });
            }
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void MaterialPropertiesView::DrawTextureSlot(
        const char* label,
        uint32_t& currentTexId, const std::string& currentTexName, const std::vector<TextureOption>& texOptions,
        uint32_t& currentSampId, const std::string& currentSampName, const std::vector<SamplerOption>& sampOptions,
        bool& changed)
    {
        std::string texLabel = std::string(label) + " Tex";
        if (Syn::UI::BeginPropertyCombo(texLabel.c_str(), currentTexName.c_str())) {
            if (ImGui::Selectable("None", currentTexId == 0xFFFFFFFF)) {
                if (currentTexId != 0xFFFFFFFF) {
                    currentTexId = 0xFFFFFFFF;
                    changed = true;
                }
            }
            for (const auto& opt : texOptions) {
                bool isSelected = (currentTexId == opt.id);
                if (ImGui::Selectable(opt.name.c_str(), isSelected)) {
                    if (currentTexId != opt.id) {
                        currentTexId = opt.id;
                        changed = true;
                    }
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            Syn::UI::EndPropertyCombo();
        }

        std::string sampLabel = std::string(label) + " Sampler";
        if (Syn::UI::BeginPropertyCombo(sampLabel.c_str(), currentSampName.c_str())) {

            if (ImGui::Selectable("Default", currentSampId == 0xFFFFFFFF)) {
                if (currentSampId != 0xFFFFFFFF) {
                    currentSampId = 0xFFFFFFFF;
                    changed = true;
                }
            }

            for (const auto& opt : sampOptions) {
                bool isSelected = (currentSampId == opt.id);
                if (ImGui::Selectable(opt.name.c_str(), isSelected)) {
                    if (currentSampId != opt.id) {
                        currentSampId = opt.id;
                        changed = true;
                    }
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            Syn::UI::EndPropertyCombo();
        }

        Syn::UI::PropertySeparator();
    }
}