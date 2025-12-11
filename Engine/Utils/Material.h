#pragma once
#include <glm/glm.hpp>
#include "Engine/EngineApi.h"
#include "Engine/Utils/ImageTexture.h"
#include "Engine/Utils/BitsetFlagged.h"

struct ENGINE_API Material : public BufferArrayIndexed, public BitsetFlagged
{
    Material();

    bool useBloom;
    glm::vec4 color;
    glm::vec2 uvScale;

    glm::vec3 emissiveColor;
    float emissiveIntensity;

    float metalness;
    float roughness;

    float aoStrength;

    std::shared_ptr<ImageTexture> albedoTexture;
    std::shared_ptr<ImageTexture> normalTexture;
    std::shared_ptr<ImageTexture> metalnessTexture;
    std::shared_ptr<ImageTexture> roughnessTexture;
    std::shared_ptr<ImageTexture> emissiveTexture;
    std::shared_ptr<ImageTexture> ambientOcclusionTexture;
};

struct ENGINE_API MaterialGPU
{
    MaterialGPU(std::shared_ptr<Material> material);

    glm::vec4 color;
    glm::vec4 emissiveColor;
    glm::vec2 uvScale;
    uint32_t albedoIndex;
    uint32_t normalIndex;
    uint32_t metallicIndex;
    uint32_t roughnessIndex;
    uint32_t emissiveIndex;
    uint32_t ambientOcclusionIndex;
    float metalness;
    float roughness;
	float aoStrength;
    uint32_t padding;
};