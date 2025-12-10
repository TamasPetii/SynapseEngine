#include "PointLightComponent.h"

PointLightComponent::PointLightComponent() :
	position(glm::vec3(0.f)),
	radius(1.f),
	weakenDistance(0.f),
	transform(glm::mat4(1))
{
	radius = 5.f;
	strength = 5.f;
	weakenDistance = 0.0f;
	falloff = LightFalloff::QUADRATIC;
}

PointLightGPU::PointLightGPU(const PointLightComponent& pointLightComponent, uint32_t shadowDenseIndex) :
	color(pointLightComponent.color),
	strength(pointLightComponent.strength),
	position(pointLightComponent.position),
	shininess(pointLightComponent.shininess),
	radius(pointLightComponent.radius),
	weakenDistance(pointLightComponent.weakenDistance),
	bitflag(0),
	shadowDenseIndex(shadowDenseIndex)
{
	bitflag |= (pointLightComponent.useShadow ? 1u : 0u) << 0; // Bit 0 = Simulate Shadow?
	bitflag |= (pointLightComponent.falloff == LightFalloff::QUADRATIC ? 1u : 0u) << 1; // Bit 1 = Quadratic/Linear
}

PointLightShadowGPU::PointLightShadowGPU(const PointLightShadowComponent& shadowComponent) :
	nearPlane(shadowComponent.nearPlane),
	farPlane(shadowComponent.farPlane),
	shadowMapSize(glm::vec2(static_cast<float>(shadowComponent.textureSize)))
{
	std::memcpy(viewProj, shadowComponent.viewProj, sizeof(viewProj));
}
