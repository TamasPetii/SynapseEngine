#include "PointLightComponent.h"

PointLightShadow::PointLightShadow()
{
}

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

PointLightGPU::PointLightGPU(const PointLightComponent& pointLightComponent) :
	color(pointLightComponent.color),
	strength(pointLightComponent.strength),
	position(pointLightComponent.position),
	shininess(pointLightComponent.shininess),
	radius(pointLightComponent.radius),
	weakenDistance(pointLightComponent.weakenDistance),
	bitflag(0)
{
	for (int i = 0; i < 6; ++i)
		shadowViewProj[i] = pointLightComponent.shadow.viewProj[i];

	bitflag |= (pointLightComponent.shadow.use ? 1u : 0u) << 0; // Bit 0 = Simulate Shadow?
	bitflag |= (pointLightComponent.falloff == LightFalloff::QUADRATIC ? 1u : 0u) << 1; // Bit 1 = Quadratic/Linear
}