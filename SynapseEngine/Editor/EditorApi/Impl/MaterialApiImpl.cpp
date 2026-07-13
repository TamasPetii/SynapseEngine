#include "MaterialApiImpl.h"
#include "EditorCore/ViewModels/MaterialWorkspace/MaterialGraph/MaterialGraphState.h"
#include "Engine/Scene/Insiders/SceneInsider.h"
#include "Engine/Component/Core/TagComponent.h"
#include "Engine/Component/Rendering/MaterialOverrideComponent.h"
#include "Engine/Logger/SynLog.h"
#include "Editor/EditorApi/EditorApiUtils.h"
#include <filesystem>

namespace Syn {

    std::vector<MaterialItemData> MaterialApiImpl::GetAllMaterials() const {
        if (!_materialManager) return {};

        std::vector<MaterialItemData> result;
        auto paths = _materialManager->GetResourcePaths();

        for (uint32_t i = 0; i < paths.size(); ++i) {
            if (_materialManager->GetEntryState(i) == ResourceState::Ready) {
                std::filesystem::path p(paths[i]);
                result.push_back({ i, p.filename().string(), paths[i] });
            }
        }
        return result;
    }

    uint32_t MaterialApiImpl::GetSelectedMaterial() const { 
        return _selectedMaterial;
    }

    void MaterialApiImpl::SetSelectedMaterial(uint32_t id) { 
        _selectedMaterial = id;
    }

    uint64_t MaterialApiImpl::GetVersion() const { 
        return _materialManager ? _materialManager->GetVersion() : 0;
    }

    std::string MaterialApiImpl::GetMaterialName(uint32_t materialId) const {
        auto mats = GetAllMaterials();
        for (const auto& m : mats) {
            if (m.id == materialId) return m.name;
        }
        return "Unknown Material";
    }

    uint32_t MaterialApiImpl::GetLinkedTexture(uint32_t materialId, uint32_t textureType) const {
        if (!_materialManager || materialId == INVALID_MATERIAL_ID) return INVALID_MATERIAL_ID;

        auto mat = _materialManager->GetResource(materialId);
        if (!mat) return INVALID_MATERIAL_ID;

        switch (static_cast<GraphPinType>(textureType)) {
        case GraphPinType::Albedo: return mat->albedoTexture;
        case GraphPinType::Normal: return mat->normalTexture;
        case GraphPinType::Metalness: return mat->metalnessTexture;
        case GraphPinType::Roughness: return mat->roughnessTexture;
        case GraphPinType::MetallicRoughness: return mat->metallicRoughnessTexture;
        case GraphPinType::Emissive: return mat->emissiveTexture;
        case GraphPinType::AmbientOcclusion: return mat->ambientOcclusionTexture;
        default: return INVALID_MATERIAL_ID;
        }
    }

    void MaterialApiImpl::LinkTextureToMaterial(uint32_t materialId, uint32_t textureType, uint32_t textureId) {
        if (!_materialManager || materialId == INVALID_MATERIAL_ID) return;

        auto mat = _materialManager->GetResource(materialId);
        if (!mat) return;

        switch (static_cast<GraphPinType>(textureType)) {
        case GraphPinType::Albedo: mat->albedoTexture = textureId; break;
        case GraphPinType::Normal: mat->normalTexture = textureId; break;
        case GraphPinType::Metalness: mat->metalnessTexture = textureId; break;
        case GraphPinType::Roughness: mat->roughnessTexture = textureId; break;
        case GraphPinType::MetallicRoughness: mat->metallicRoughnessTexture = textureId; break;
        case GraphPinType::Emissive: mat->emissiveTexture = textureId; break;
        case GraphPinType::AmbientOcclusion: mat->ambientOcclusionTexture = textureId; break;
        default: break;
        }
    }

    void MaterialApiImpl::UnlinkTextureFromMaterial(uint32_t materialId, uint32_t textureType) {
        LinkTextureToMaterial(materialId, textureType, 0xFFFFFFFF);
    }

    bool MaterialApiImpl::GetMaterialData(uint32_t materialId, Material& outMaterial) const {
        if (!_materialManager || materialId == INVALID_MATERIAL_ID) return false;

        auto resource = _materialManager->GetResource(materialId);
        if (resource) {
            outMaterial = *resource;
            return true;
        }
        return false;
    }

    void MaterialApiImpl::UpdateMaterialData(uint32_t materialId, const Material& material) {
        if (!_materialManager || materialId == INVALID_MATERIAL_ID) return;
        auto resource = _materialManager->GetResource(materialId);

        if (resource) {
            *resource = material;
            _materialManager->MarkDirty(materialId);
        }
    }

    void MaterialApiImpl::ApplyMaterialToPreviewObjects(uint32_t materialId) {
        if (!_sceneManager) return;
        auto scene = _sceneManager->GetActiveScene();
        if (!scene) return;

        auto& registry = SceneInsider::GetRegistry(*scene, SceneInsider::GetKey());

        auto tagPool = registry.GetPool<TagComponent>();
        if (!tagPool) return;

        for (EntityID entity : tagPool->GetDenseEntities()) {
            if (registry.HasComponent<MaterialOverrideComponent>(entity)) {
                const auto& tag = tagPool->Get(entity);

                if (tag.tag == "Preview") {

                    EditorApiUtils::ModifyComponent<MaterialOverrideComponent>(
                        _sceneManager,
                        entity,
                        [materialId](auto& matOverride, auto pool) {
                            matOverride.materials.clear();
                            if (materialId != 0xFFFFFFFF) {
                                matOverride.materials.push_back(materialId);
                            }
                        }
                    );
                }
            }
        }

        Info("MaterialApiImpl: Applied material {} to preview objects.", materialId);
    }
}