#pragma once
#include "BaseComponents/Component.h"
#include "BaseComponents/Light.h"
#include "BaseComponents/VersionIndexed.h"
#include "BaseComponents/FrustumCullable.h"
#include "Engine/Renderable/Instanceable.h"

#include <glm/glm.hpp>
#include <memory>
#include <array>

constexpr float defaultPointLightRadius = 1.f;

class PointLightSystem;

struct ENGINE_API PointLightComponent : public Light, public Component, public FrustumCullable
{
	PointLightComponent();

	glm::vec3 position; //Mapped to transform component translation!
	float radius; //Mapped to transform component scale (max value)
	float weakenDistance;
private:
	glm::mat4 transform; //Cannot use transform component becouse of the scale can be different on xyz!
	friend class PointLightSystem;
};

struct ENGINE_API PointLightGPU
{
	PointLightGPU(const PointLightComponent& pointLightComponent, uint32_t shadowDenseIndex = UINT32_MAX);

	glm::vec3 color;
	float strength;
	glm::vec3 position;
	float shininess;
	float radius;
	float weakenDistance;
	uint32_t bitflag;
	uint32_t shadowDenseIndex;
};

struct ENGINE_API PointLightShadowComponent : public LightShadow, public Component
{
	glm::mat4 viewProj[6];
};

struct ENGINE_API PointLightShadowGPU
{
	PointLightShadowGPU(const PointLightShadowComponent& shadowComponent);

	float nearPlane;
	float farPlane;
	glm::vec2 shadowMapSize;
	glm::mat4 viewProj[6];
};