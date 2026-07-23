#include "MaterialGraphView.h"
#include "Editor/Manager/EditorIcons.h" 
#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace Syn {

    MaterialGraphView::MaterialGraphView() {
        ed::Config config;
        config.SettingsFile = "Synapse_MaterialGraph.json";
        _context = ed::CreateEditor(&config);
    }

    MaterialGraphView::~MaterialGraphView() {
        if (_context) {
            ed::DestroyEditor(_context);
        }
    }

    MaterialGraphView::MaterialGraphView(MaterialGraphView&& other) noexcept
        : _context(other._context)
    {
        other._context = nullptr;
    }

    MaterialGraphView& MaterialGraphView::operator=(MaterialGraphView&& other) noexcept {
        if (this != &other) {
            if (_context) ed::DestroyEditor(_context);
            _context = other._context;
            other._context = nullptr;
        }
        return *this;
    }

    void MaterialGraphView::Draw(MaterialGraphViewModel& vm) {
        const auto& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        bool isVisible = ImGui::Begin(SYN_ICON_PROJECT_DIAGRAM " Material Graph", nullptr, windowFlags);
        ImGui::PopStyleVar();

        if (isVisible) {
            ed::SetCurrentEditor(_context);

            ImVec2 canvasSize = ImGui::GetContentRegionAvail();
            if (canvasSize.x <= 0.0f) canvasSize.x = 1.0f;
            if (canvasSize.y <= 0.0f) canvasSize.y = 1.0f;

            ed::Begin("Material Editor Canvas", canvasSize);

            if (!state.isMaterialSelected) {
                ImGui::SetCursorPos(ImVec2(canvasSize.x * 0.5f - 80.0f, canvasSize.y * 0.5f));
                ImGui::TextDisabled("No Material Selected");
            }
            else {
                for (const auto& node : state.nodes) {
                    if (node.type == GraphNodeType::Material) {
                        DrawMaterialNode(node);
                    }
                    else if (node.type == GraphNodeType::Texture) {
                        DrawTextureNode(node);
                    }
                }

                for (const auto& link : state.links) {
                    ed::Link(ed::LinkId(link.id), ed::PinId(link.startPinId), ed::PinId(link.endPinId));
                }

                if (ed::BeginCreate()) {
                    ed::PinId inputPinId, outputPinId;
                    if (ed::QueryNewLink(&inputPinId, &outputPinId)) {
                        if (inputPinId && outputPinId) {
                            if (ed::AcceptNewItem(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), 2.0f)) {
                                GraphID startId = static_cast<GraphID>(inputPinId.Get());
                                GraphID endId = static_cast<GraphID>(outputPinId.Get());
                                vm.Dispatch(CreateLinkIntent{ startId, endId });
                            }
                        }
                    }
                }
                ed::EndCreate();

                if (ed::BeginDelete()) {
                    ed::LinkId deletedLinkId;
                    while (ed::QueryDeletedLink(&deletedLinkId)) {
                        if (ed::AcceptDeletedItem()) {
                            vm.Dispatch(DeleteLinkIntent{ static_cast<GraphID>(deletedLinkId.Get()) });
                        }
                    }
                }
                ed::EndDelete();
            }

            ed::End();
            ed::SetCurrentEditor(nullptr);
        }

        ImGui::End();
    }

    void MaterialGraphView::DrawMaterialNode(const GraphNodeData& node) {
        ed::BeginNode(ed::NodeId(node.id));

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
        ImGui::TextUnformatted((SYN_ICON_BRUSH " " + node.name).c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImVec2 previewSize(256.0f, 256.0f);
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        drawList->AddRectFilled(cursorPos, ImVec2(cursorPos.x + previewSize.x, cursorPos.y + previewSize.y), IM_COL32(25, 25, 25, 255), 4.0f);

        const char* previewText = "Material\nPreview";
        ImVec2 textSize = ImGui::CalcTextSize(previewText);
        drawList->AddText(ImVec2(cursorPos.x + (previewSize.x - textSize.x) * 0.5f, cursorPos.y + (previewSize.y - textSize.y) * 0.5f), IM_COL32(120, 120, 120, 255), previewText);
        ImGui::Dummy(previewSize);

        ImGui::Spacing();

        for (const auto& pin : node.pins) {
            ed::BeginPin(ed::PinId(pin.id), ed::PinKind::Input);
            ImGui::Text("-> %s", GetPinName(pin.type));
            ed::EndPin();
        }

        ed::EndNode();
    }

    void MaterialGraphView::DrawTextureNode(const GraphNodeData& node) {
        ed::BeginNode(ed::NodeId(node.id));

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
        ImGui::TextUnformatted((SYN_ICON_IMAGE " " + node.name).c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImVec2 previewSize(256.0f, 256.0f);
        if (node.textureHandle != InvalidTextureHandle) {
            ImGui::Image(node.textureHandle, previewSize);
        }
        else {
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(cursorPos, ImVec2(cursorPos.x + previewSize.x, cursorPos.y + previewSize.y), IM_COL32(25, 25, 25, 255), 4.0f);
            const char* previewText = "No Image";
            ImVec2 textSize = ImGui::CalcTextSize(previewText);
            drawList->AddText(ImVec2(cursorPos.x + (previewSize.x - textSize.x) * 0.5f, cursorPos.y + (previewSize.y - textSize.y) * 0.5f), IM_COL32(255, 100, 100, 255), previewText);
            ImGui::Dummy(previewSize);
        }

        ImGui::Spacing();

        for (const auto& pin : node.pins) {
            ImVec2 pinTextSize = ImGui::CalcTextSize("RGB ->");

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + previewSize.x - pinTextSize.x);

            ed::BeginPin(ed::PinId(pin.id), ed::PinKind::Output);
            ImGui::Text("RGB ->");
            ed::EndPin();
        }

        ed::EndNode();
    }

    const char* MaterialGraphView::GetPinName(GraphPinType type) {
        switch (type) {
        case GraphPinType::Albedo: return "Albedo";
        case GraphPinType::Normal: return "Normal";
        case GraphPinType::Metalness: return "Metalness";
        case GraphPinType::Roughness: return "Roughness";
        case GraphPinType::MetallicRoughness: return "MetallicRoughness";
        case GraphPinType::Emissive: return "Emissive";
        case GraphPinType::AmbientOcclusion: return "Ambient Occlusion";
        case GraphPinType::Clearcoat: return "Clearcoat";
        case GraphPinType::ClearcoatRoughness: return "CC Roughness";
        case GraphPinType::ClearcoatNormal: return "CC Normal";
        case GraphPinType::Specular: return "Specular";
        case GraphPinType::SpecularColor: return "Specular Color";

        default: return "Unknown";
        }
    }
}