#include "MaterialGraphView.h"
#include <imgui.h>
#include <imgui_node_editor.h>

namespace ed = ax::NodeEditor;

namespace Syn {

    MaterialGraphView::MaterialGraphView() {
        ed::Config config;
        config.SettingsFile = nullptr;
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
            if (_context) {
                ed::DestroyEditor(_context);
            }

            _context = other._context;
            other._context = nullptr;
        }
        return *this;
    }

    void MaterialGraphView::Draw(MaterialGraphViewModel& vm) {
        const auto& state = vm.GetState();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        bool isVisible = ImGui::Begin("Material Graph", nullptr, windowFlags);

        ImGui::PopStyleVar();

        if (isVisible) {
            ImVec2 startPos = ImGui::GetCursorStartPos();
            ImGui::SetCursorPos(startPos);

            ed::SetCurrentEditor(_context);

            ImVec2 canvasSize = ImGui::GetContentRegionAvail();
            if (canvasSize.x <= 0.0f) canvasSize.x = 1.0f;
            if (canvasSize.y <= 0.0f) canvasSize.y = 1.0f;

            ed::Begin("Material Editor Canvas", canvasSize);

            for (const auto& node : state.nodes) {
                ed::BeginNode(ed::NodeId(node.id));

                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
                ImGui::TextUnformatted(node.name.c_str());
                ImGui::PopStyleColor();

                if (node.type == GraphNodeType::Material) {
                    for (const auto& pin : node.pins) {
                        ed::BeginPin(ed::PinId(pin.id), ed::PinKind::Input);
                        ImGui::Text("-> %s", GetPinName(pin.type));
                        ed::EndPin();
                    }
                }
                else if (node.type == GraphNodeType::Texture) {
                    for (const auto& pin : node.pins) {
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);
                        ed::BeginPin(ed::PinId(pin.id), ed::PinKind::Output);
                        ImGui::Text("RGB ->");
                        ed::EndPin();
                    }
                }

                ed::EndNode();
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

            ed::End();
            ed::SetCurrentEditor(nullptr);
        }

        ImGui::End();
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
        default: return "Unknown";
        }
    }
}