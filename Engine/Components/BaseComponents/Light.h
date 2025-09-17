#pragma once
#include "VersionIndexed.h"

#include "Engine/Vulkan/FrameBuffer.h"
#include "Engine/Config.h"
#include <glm/glm.hpp>
#include <memory>
#include <array>

enum LightFalloff
{
	LINEAR = 0,
	QUADRATIC = 1
};

struct ENGINE_API LightShadowFrameBuffer : public VersionIndexed
{
	std::shared_ptr<Vk::FrameBuffer> frameBuffer = nullptr;
};

struct ENGINE_API LightShadow : public VersionIndexed
{
	LightShadow();

	bool use;
	uint32_t textureSize;
	uint32_t updateFrequency;
	std::array<LightShadowFrameBuffer, GlobalConfig::FrameConfig::maxFramesInFlights> frameBuffers;
};

struct ENGINE_API Light
{
	Light();

	glm::vec3 color;
	float strength;
	float shininess;
	LightFalloff falloff;
};
