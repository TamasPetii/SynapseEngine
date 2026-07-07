#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "Engine/Material/Material.h"
//Todo: Domain Material!!

namespace Syn 
{
    constexpr uint32_t INVALID_MATERIAL_ID = 0xFFFFFFFF;

    struct MaterialItemData {
        uint32_t id;
        std::string name;
        std::string path;
    };

    class IMaterialApi {
    public:
        virtual ~IMaterialApi() = default;

        virtual std::vector<MaterialItemData> GetAllMaterials() const = 0;
        virtual uint32_t GetSelectedMaterial() const = 0;
        virtual void SetSelectedMaterial(uint32_t id) = 0;
        virtual uint64_t GetVersion() const = 0;

        virtual std::string GetMaterialName(uint32_t materialId) const = 0;
        virtual uint32_t GetLinkedTexture(uint32_t materialId, uint32_t textureType) const = 0;
        virtual void LinkTextureToMaterial(uint32_t materialId, uint32_t textureType, uint32_t textureId) = 0;
        virtual void UnlinkTextureFromMaterial(uint32_t materialId, uint32_t textureType) = 0;

        virtual bool GetMaterialData(uint32_t materialId, Material& outMaterial) const = 0;
        virtual void UpdateMaterialData(uint32_t materialId, const Material& material) = 0;

        virtual void ApplyMaterialToPreviewObjects(uint32_t materialId) = 0;
    };
}