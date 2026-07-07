#pragma once
#include "EditorCore/Api/IMaterialApi.h"
#include "Engine/Material/MaterialManager.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    enum class GraphPinType;

    class MaterialApiImpl : public IMaterialApi {
    public:
        MaterialApiImpl(MaterialManager* materialManager, SceneManager* sceneManager)
            : _materialManager(materialManager), _sceneManager(sceneManager) {}

        std::vector<MaterialItemData> GetAllMaterials() const override;
        uint32_t GetSelectedMaterial() const override;
        void SetSelectedMaterial(uint32_t id) override;
        uint64_t GetVersion() const override;

        std::string GetMaterialName(uint32_t materialId) const override;
        uint32_t GetLinkedTexture(uint32_t materialId, uint32_t textureType) const override;

        void LinkTextureToMaterial(uint32_t materialId, uint32_t textureType, uint32_t textureId) override;
        void UnlinkTextureFromMaterial(uint32_t materialId, uint32_t textureType) override;

        bool GetMaterialData(uint32_t materialId, Material& outMaterial) const override;
        void UpdateMaterialData(uint32_t materialId, const Material& material) override;

        void ApplyMaterialToPreviewObjects(uint32_t materialId) override;
    private:
        SceneManager* _sceneManager;
        MaterialManager* _materialManager;
        uint32_t _selectedMaterial = INVALID_MATERIAL_ID;
    };
}