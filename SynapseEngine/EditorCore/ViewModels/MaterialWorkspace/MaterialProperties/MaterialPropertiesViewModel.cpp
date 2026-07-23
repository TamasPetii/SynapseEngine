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

            _state.availableSamplers.clear();
            auto samps = _textureApi ? _textureApi->GetAllSamplers() : std::vector<SamplerItemData>();
            for (const auto& s : samps) {
                _state.availableSamplers.push_back({ s.id, s.name });
            }

            auto getTexName = [&](uint32_t id) -> std::string {
                if (id == 0xFFFFFFFF) return "None";
                for (const auto& t : texs) {
                    if (t.id == id) return t.name;
                }
                return "Unknown";
                };

            auto getSamplerName = [&](uint32_t id) -> std::string {
                if (id == 0xFFFFFFFF || id == 0xFF) return "Default";
                for (const auto& s : samps) {
                    if (s.id == id) return s.name;
                }
                return "Unknown";
                };

            _state.albedoName = getTexName(matData.albedoTexture);
            _state.albedoSamplerName = getSamplerName(matData.albedoSampler);

            _state.normalName = getTexName(matData.normalTexture);
            _state.normalSamplerName = getSamplerName(matData.normalSampler);

            _state.metalnessName = getTexName(matData.metalnessTexture);
            _state.metalnessSamplerName = getSamplerName(matData.metalnessSampler);

            _state.roughnessName = getTexName(matData.roughnessTexture);
            _state.roughnessSamplerName = getSamplerName(matData.roughnessSampler);

            _state.metallicRoughnessName = getTexName(matData.metallicRoughnessTexture);
            _state.metallicRoughnessSamplerName = getSamplerName(matData.metallicRoughnessSampler);

            _state.emissiveName = getTexName(matData.emissiveTexture);
            _state.emissiveSamplerName = getSamplerName(matData.emissiveSampler);

            _state.aoName = getTexName(matData.ambientOcclusionTexture);
            _state.aoSamplerName = getSamplerName(matData.ambientOcclusionSampler);

            _state.opacityName = getTexName(matData.opacityTexture);
            _state.opacitySamplerName = getSamplerName(matData.opacitySampler);

            _state.clearcoatName = getTexName(matData.clearcoatTexture);
            _state.clearcoatSamplerName = getSamplerName(matData.clearcoatSampler);

            _state.clearcoatRoughnessName = getTexName(matData.clearcoatRoughnessTexture);
            _state.clearcoatRoughnessSamplerName = getSamplerName(matData.clearcoatRoughnessSampler);

            _state.clearcoatNormalName = getTexName(matData.clearcoatNormalTexture);
            _state.clearcoatNormalSamplerName = getSamplerName(matData.clearcoatNormalSampler);

            _state.specularName = getTexName(matData.specularTexture);
            _state.specularSamplerName = getSamplerName(matData.specularSampler);

            _state.specularColorName = getTexName(matData.specularColorTexture);
            _state.specularColorSamplerName = getSamplerName(matData.specularColorSampler);
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