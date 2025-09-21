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
	float metalness;
	float roughness;
	std::shared_ptr<ImageTexture> albedoTexture;
	std::shared_ptr<ImageTexture> normalTexture;
	std::shared_ptr<ImageTexture> metalnessTexture;
	std::shared_ptr<ImageTexture> roughnessTexture;
};

struct ENGINE_API MaterialGPU
{
	MaterialGPU(std::shared_ptr<Material> material);

	glm::vec4 color;
	glm::vec2 uvScale;
	glm::vec2 bloom;
	uint32_t albedoIndex;
	uint32_t normalIndex;
	uint32_t metallicIndex;
	uint32_t roughnessIndex;
	float metalness;
	float roughness;
	glm::vec2 padding;
};