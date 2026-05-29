#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace Syn 
{
    struct MaterialApiDesc {
        uint32_t id;
        std::string name;
    };

    struct TextureApiDesc {
        uint32_t id;
        std::string name;
    };

    class IMaterialAPI {
    public:
        virtual ~IMaterialAPI() = default;

        virtual std::vector<MaterialApiDesc> GetAllMaterials() const = 0;
        virtual std::vector<TextureApiDesc> GetAllTextures() const = 0;

        virtual void LinkTextureToMaterial(uint32_t materialId, uint32_t textureType, uint32_t textureId) = 0;
        virtual void UnlinkTextureFromMaterial(uint32_t materialId, uint32_t textureType) = 0;
    };
}