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

#include "TextureGraphView.h"
#include "Editor/Manager/EditorIcons.h"
#include <imgui.h>
#include <imgui_node_editor.h>
#include <algorithm>

namespace ed = ax::NodeEditor;

namespace Syn {

    TextureGraphView::TextureGraphView() {
        ed::Config config;
        config.SettingsFile = "Synapse_TextureGraph.json";
        _context = ed::CreateEditor(&config);
    }

    TextureGraphView::~TextureGraphView() {
        if (_context) ed::DestroyEditor(_context);
    }

    TextureGraphView::TextureGraphView(TextureGraphView&& other) noexcept : _context(other._context) {
        other._context = nullptr;
    }

    TextureGraphView& TextureGraphView::operator=(TextureGraphView&& other) noexcept {
        if (this != &other) {
            if (_context) ed::DestroyEditor(_context);
            _context = other._context;
            other._context = nullptr;
        }
        return *this;
    }

    void TextureGraphView::Draw(TextureGraphViewModel& vm) {
        const auto& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        bool isVisible = ImGui::Begin(SYN_ICON_PROJECT_DIAGRAM " Texture Graph", nullptr, windowFlags);
        ImGui::PopStyleVar();

        if (isVisible) {
            ed::SetCurrentEditor(_context);

            ImVec2 canvasSize = ImGui::GetContentRegionAvail();
            if (canvasSize.x <= 0.0f) canvasSize.x = 1.0f;
            if (canvasSize.y <= 0.0f) canvasSize.y = 1.0f;

            ed::Begin("Texture Node Canvas", canvasSize);

            if (state.previewNode.isVisible) {
                ed::BeginNode(ed::NodeId(1));

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
                ImGui::Text(SYN_ICON_IMAGE " %s", state.previewNode.name.empty() ? "Selected Texture" : state.previewNode.name.c_str());
                ImGui::PopStyleColor();
                ImGui::Dummy(ImVec2(0, 4.0f));

                float maxPreviewSize = 512.0f;
                ImVec2 imageSize(512.0f, 512.0f);
                if (state.previewNode.width > 0 && state.previewNode.height > 0) {
                    float aspect = (float)state.previewNode.width / (float)state.previewNode.height;
                    if (aspect > 1.0f) {
                        imageSize.x = maxPreviewSize;
                        imageSize.y = maxPreviewSize / aspect;
                    }
                    else {
                        imageSize.y = maxPreviewSize;
                        imageSize.x = maxPreviewSize * aspect;
                    }
                }

                if (state.previewNode.textureHandle != InvalidTextureHandle) {
                    ImGui::Image(state.previewNode.textureHandle, imageSize);
                }
                else {
                    ImGui::Button("No Preview\nAvailable", imageSize);
                }

                ImGui::Dummy(ImVec2(0, 8.0f));

                ed::EndNode();
            }

            ed::End();
            ed::SetCurrentEditor(nullptr);
        }

        ImGui::End();
    }
}