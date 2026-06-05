#include "MaterialGraphViewModel.h"
#include <algorithm>

namespace Syn {

    MaterialGraphViewModel::MaterialGraphViewModel(IMaterialApi* materialApi)
        : _materialApi(materialApi)
    {
        BuildGraphFromEngine();
    }

    void MaterialGraphViewModel::SyncWithEngine() {

    }

    void MaterialGraphViewModel::Dispatch(const MaterialGraphIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, CreateLinkIntent>) {
                HandleCreateLink(arg);
            }
            else if constexpr (std::is_same_v<T, DeleteLinkIntent>) {
                HandleDeleteLink(arg);
            }
            }, intent);
    }

    void MaterialGraphViewModel::BuildGraphFromEngine() {
        _state.nodes.clear();
        _state.links.clear();
        _state.nextId = 1;

        auto materials = _materialApi->GetAllMaterials();
        for (const auto& mat : materials) {
            GraphID matNodeId = _state.nextId++;
            GraphNodeData matNode{ matNodeId, GraphNodeType::Material, mat.id, mat.name, {} };

            matNode.pins.push_back({ _state.nextId++, matNodeId, GraphPinType::Albedo, true });
            matNode.pins.push_back({ _state.nextId++, matNodeId, GraphPinType::Normal, true });
            matNode.pins.push_back({ _state.nextId++, matNodeId, GraphPinType::Metalness, true });
            matNode.pins.push_back({ _state.nextId++, matNodeId, GraphPinType::Roughness, true });
            matNode.pins.push_back({ _state.nextId++, matNodeId, GraphPinType::Emissive, true });
            matNode.pins.push_back({ _state.nextId++, matNodeId, GraphPinType::AmbientOcclusion, true });
            _state.nodes.push_back(matNode);
        }

        auto textures = _materialApi->GetAllTextures();

        for (const auto& tex : textures) {
            GraphID texNodeId = _state.nextId++;
            GraphNodeData texNode{ texNodeId, GraphNodeType::Texture, tex.id, tex.name, {} };

            texNode.pins.push_back({ _state.nextId++, texNodeId, GraphPinType::TextureOutput, false });
            _state.nodes.push_back(texNode);
        }
    }

    void MaterialGraphViewModel::HandleCreateLink(const CreateLinkIntent& intent) {
        const GraphPinData* startPin = nullptr;
        const GraphNodeData* startNode = nullptr;

        const GraphPinData* endPin = nullptr;
        const GraphNodeData* endNode = nullptr;

        for (const auto& node : _state.nodes) {
            for (const auto& pin : node.pins) {
                if (pin.id == intent.startPinId) {
                    startPin = &pin;
                    startNode = &node;
                }
                if (pin.id == intent.endPinId) {
                    endPin = &pin;
                    endNode = &node;
                }
            }
        }

        if (startNode && endNode && startPin && endPin) {
            if (startNode->type == GraphNodeType::Texture && 
                endNode->type == GraphNodeType::Material) 
            {
                _materialApi->LinkTextureToMaterial(
                    endNode->engineResourceId,
                    static_cast<uint32_t>(endPin->type),
                    startNode->engineResourceId
                );

                _state.links.push_back({ _state.nextId++, intent.startPinId, intent.endPinId });
            }
        }
    }

    void MaterialGraphViewModel::HandleDeleteLink(const DeleteLinkIntent& intent) {
        auto it = std::find_if(_state.links.begin(), _state.links.end(), [&](const GraphLinkData& link) {
            return link.id == intent.linkId;
            });

        if (it != _state.links.end()) {
            GraphID endPinId = it->endPinId;

            for (const auto& node : _state.nodes) {
                if (node.type == GraphNodeType::Material) {
                    for (const auto& pin : node.pins) {
                        if (pin.id == endPinId) {
                            _materialApi->UnlinkTextureFromMaterial(node.engineResourceId, static_cast<uint32_t>(pin.type));
                            break;
                        }
                    }
                }
            }

            _state.links.erase(it);
        }
    }
}