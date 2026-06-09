#pragma once
#include "EditorCore/Api/IMaterialApi.h"
#include "Engine/Engine.h"

namespace Syn {
    class MaterialApiImpl : public IMaterialApi {
    public:
        MaterialApiImpl(Engine* engine) : _engine(engine) {}
        std::vector<MaterialApiDesc> GetAllMaterials() const override;
        std::vector<TextureApiDesc> GetAllTextures() const override;
        void LinkTextureToMaterial(uint32_t materialId, uint32_t textureType, uint32_t textureId) override;
        void UnlinkTextureFromMaterial(uint32_t materialId, uint32_t textureType) override;
    private:
        Engine* _engine;
    };
}