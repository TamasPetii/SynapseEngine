#pragma once
#include "VersionIndexed.h"

#include "Engine/Vulkan/FrameBuffer.h"
#include "Engine/Config.h"
#include <glm/glm.hpp>
#include <memory>
#include <array>

constexpr glm::vec4 defaultLightPosition = glm::vec4(0.f, 0.f, 0.f, 1.f);

enum LightFalloff
{
	LINEAR = 0,
	QUADRATIC = 1
};

struct ENGINE_API LightShadowFrameBuffer : public VersionIndexed
{
	std::shared_ptr<Vk::FrameBuffer> frameBuffer = nullptr;
};

struct ENGINE_API LightShadow
{
	LightShadow();

	float nearPlane = 0.1f;
	float farPlane = 0.1f;
	uint32_t textureSize;
	uint32_t updateFrequency;
	std::array<LightShadowFrameBuffer, GlobalConfig::FrameConfig::maxFramesInFlights> frameBuffers;
};

struct ENGINE_API Light
{
	Light();

	bool useShadow;
	glm::vec3 color;
	float strength;
	float shininess;
	LightFalloff falloff;
};
