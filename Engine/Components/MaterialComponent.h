#pragma once
#include "BaseComponents/Component.h"
#include <glm/glm.hpp>
#include "Engine/Utils/ImageTexture.h"

struct ENGINE_API MaterialComponent : public Component
{
	MaterialComponent();

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

struct ENGINE_API MaterialComponentGPU
{
	MaterialComponentGPU(const MaterialComponent& component);

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