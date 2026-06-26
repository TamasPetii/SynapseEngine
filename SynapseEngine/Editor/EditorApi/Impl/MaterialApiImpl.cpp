#include "MaterialApiImpl.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Image/ImageManager.h"
#include "Engine/Logger/SynLog.h"

namespace Syn {
    std::vector<MaterialApiDesc> MaterialApiImpl::GetAllMaterials() const {
        std::vector<MaterialApiDesc> result;
        auto materialManager = _engine->GetMaterialManager();
        if (!materialManager) return result;

        for (const auto& path : materialManager->GetResourcePaths()) {
            result.push_back({ materialManager->GetResourceIndex(path), path });
        }
        return result;
    }

    std::vector<TextureApiDesc> MaterialApiImpl::GetAllTextures() const {
        std::vector<TextureApiDesc> result;
        auto imageManager = _engine->GetImageManager();
        if (!imageManager) return result;

        for (const auto& path : imageManager->GetResourcePaths()) {
            result.push_back({ imageManager->GetResourceIndex(path), path });
        }
        return result;
    }

    void MaterialApiImpl::LinkTextureToMaterial(uint32_t materialId, uint32_t textureType, uint32_t textureId) {
        auto materialManager = _engine->GetMaterialManager();
        if (!materialManager) return;
        auto material = materialManager->GetResource(materialId);
        if (!material) return;

        switch (textureType) {
            case 0: material->albedoTexture = textureId; break;
            case 1: material->normalTexture = textureId; break;
            case 2: material->metalnessTexture = textureId; break;
            case 3: material->roughnessTexture = textureId; break;
            case 4: material->metallicRoughnessTexture = textureId; break;
            case 5: material->emissiveTexture = textureId; break;
            case 6: material->ambientOcclusionTexture = textureId; break;
            default:
                Syn::Warning("MaterialApiImpl: Ismeretlen textúra slot ({}) a {} azonosítójú materialhoz!", textureType, materialId);
                return;
        }
        Syn::Info("MaterialApiImpl: Textúra ({}) bekötve a Material ({}) {} slotjába.", textureId, materialId, textureType);
    }

    void MaterialApiImpl::UnlinkTextureFromMaterial(uint32_t materialId, uint32_t textureType) {
        auto materialManager = _engine->GetMaterialManager();
        if (!materialManager) return;
        auto material = materialManager->GetResource(materialId);
        if (!material) return;

        switch (textureType) {
            case 0: material->albedoTexture = UINT32_MAX; break;
            case 1: material->normalTexture = UINT32_MAX; break;
            case 2: material->metalnessTexture = UINT32_MAX; break;
            case 3: material->roughnessTexture = UINT32_MAX; break;
            case 4: material->metallicRoughnessTexture = UINT32_MAX; break;
            case 5: material->emissiveTexture = UINT32_MAX; break;
            case 6: material->ambientOcclusionTexture = UINT32_MAX; break;
        }
        Syn::Info("MaterialApiImpl: Textúra kikötve a Material ({}) {} slotjából.", materialId, textureType);
    }
}