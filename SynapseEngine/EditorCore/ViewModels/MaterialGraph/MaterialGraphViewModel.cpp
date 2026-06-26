#include "MaterialGraphViewModel.h"
#include <algorithm>

namespace Syn {

    MaterialGraphViewModel::MaterialGraphViewModel(IMaterialApi* materialApi, ITextureApi* textureApi)
        : _materialApi(materialApi), _textureApi(textureApi)
    {}

    void MaterialGraphViewModel::SyncWithEngine() {
        if (!_materialApi) return;

        uint32_t currentSel = _materialApi->GetSelectedMaterial();
        uint64_t currentVer = _materialApi->GetVersion();

        if (currentSel != _lastSelectedMaterial || currentVer != _lastEngineVersion) {
            RebuildGraphForSelectedMaterial(currentSel);
            _lastSelectedMaterial = currentSel;
            _lastEngineVersion = currentVer;
        }
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

    void MaterialGraphViewModel::RebuildGraphForSelectedMaterial(uint32_t materialId) {
        _state.nodes.clear();
        _state.links.clear();
        _state.nextId = 1;

        if (materialId == INVALID_MATERIAL_ID) {
            _state.isMaterialSelected = false;
            return;
        }

        _state.isMaterialSelected = true;
        _state.selectedMaterialId = materialId;
        _state.selectedMaterialName = _materialApi->GetMaterialName(materialId);

        GraphID matNodeId = _state.nextId++;
        GraphNodeData matNode{ matNodeId, GraphNodeType::Material, materialId, _state.selectedMaterialName, {} };

        std::vector<GraphPinType> pinTypes = {
            GraphPinType::Albedo, GraphPinType::Normal, GraphPinType::Metalness,
            GraphPinType::Roughness, GraphPinType::MetallicRoughness,
            GraphPinType::Emissive, GraphPinType::AmbientOcclusion
        };

        auto allTextures = _textureApi ? _textureApi->GetAllTextures() : std::vector<TextureItemData>();

        for (auto pType : pinTypes) {
            GraphID pinId = _state.nextId++;
            matNode.pins.push_back({ pinId, matNodeId, pType, true });

            uint32_t linkedTexId = _materialApi->GetLinkedTexture(materialId, static_cast<uint32_t>(pType));

            if (linkedTexId != 0xFFFFFFFF) {

                std::string texName = "Unknown Texture";
                for (const auto& t : allTextures) {
                    if (t.id == linkedTexId) {
                        texName = t.name;
                        break;
                    }
                }

                GraphID texNodeId = _state.nextId++;
                GraphNodeData texNode{ texNodeId, GraphNodeType::Texture, linkedTexId, texName, {} };

                if (_textureApi) {
                    texNode.textureHandle = _textureApi->GetTextureHandle(linkedTexId);
                }

                GraphID texOutPinId = _state.nextId++;
                texNode.pins.push_back({ texOutPinId, texNodeId, GraphPinType::TextureOutput, false });

                _state.nodes.push_back(texNode);
                _state.links.push_back({ _state.nextId++, texOutPinId, pinId });
            }
        }

        matNode.textureHandle = InvalidTextureHandle;
        _state.nodes.push_back(matNode);
    }

    void MaterialGraphViewModel::HandleCreateLink(const CreateLinkIntent& intent) {
        const GraphPinData* startPin = nullptr; const GraphNodeData* startNode = nullptr;
        const GraphPinData* endPin = nullptr;   const GraphNodeData* endNode = nullptr;

        for (const auto& node : _state.nodes) {
            for (const auto& pin : node.pins) {
                if (pin.id == intent.startPinId) { startPin = &pin; startNode = &node; }
                if (pin.id == intent.endPinId) { endPin = &pin;   endNode = &node; }
            }
        }

        if (startNode && endNode && startPin && endPin) {
            if (startNode->type == GraphNodeType::Texture && endNode->type == GraphNodeType::Material) {
                _materialApi->LinkTextureToMaterial(
                    endNode->engineResourceId,
                    static_cast<uint32_t>(endPin->type),
                    startNode->engineResourceId
                );
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
        }
    }
}