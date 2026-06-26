#include "MaterialPropertiesViewModel.h"

namespace Syn {
    MaterialPropertiesViewModel::MaterialPropertiesViewModel(IMaterialApi* materialApi, ITextureApi* textureApi)
        : _materialApi(materialApi), _textureApi(textureApi)
    {}

    void MaterialPropertiesViewModel::SyncWithEngine() {
        if (!_materialApi) return;

        uint32_t selectedId = _materialApi->GetSelectedMaterial();

        if (selectedId == 0xFFFFFFFF) {
            _state.hasSelection = false;
            return;
        }

        Material matData;
        if (_materialApi->GetMaterialData(selectedId, matData)) {
            _state.hasSelection = true;
            _state.selectedMaterialId = selectedId;
            _state.materialName = _materialApi->GetMaterialName(selectedId);
            _state.materialData = matData;

            _state.availableTextures.clear();
            auto texs = _textureApi ? _textureApi->GetAllTextures() : std::vector<TextureItemData>();
            for (const auto& t : texs) {
                _state.availableTextures.push_back({ t.id, t.name });
            }

            auto getTexName = [&](uint32_t id) -> std::string {
                if (id == 0xFFFFFFFF) return "None";
                for (const auto& t : texs) {
                    if (t.id == id) return t.name;
                }
                return "Unknown";
                };

            _state.albedoName = getTexName(matData.albedoTexture);
            _state.normalName = getTexName(matData.normalTexture);
            _state.metalnessName = getTexName(matData.metalnessTexture);
            _state.roughnessName = getTexName(matData.roughnessTexture);
            _state.metallicRoughnessName = getTexName(matData.metallicRoughnessTexture);
            _state.emissiveName = getTexName(matData.emissiveTexture);
            _state.aoName = getTexName(matData.ambientOcclusionTexture);
        }
        else {
            _state.hasSelection = false;
        }
    }

    void MaterialPropertiesViewModel::Dispatch(const MaterialPropertiesIntent& intent) {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, UpdateMaterialPropertyIntent>) {
                if (_state.hasSelection) {
                    _materialApi->UpdateMaterialData(_state.selectedMaterialId, arg.updatedMaterial);
                    _state.materialData = arg.updatedMaterial;
                }
            }
            }, intent);
    }
}