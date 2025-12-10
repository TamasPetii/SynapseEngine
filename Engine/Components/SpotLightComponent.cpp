#include "SpotLightComponent.h"

SpotLightComponent::SpotLightComponent() :
	position(glm::vec3(0.f)),
	transform(glm::mat4(1))
{
	strength = 10.f;
}

SpotLightGPU::SpotLightGPU(const SpotLightComponent& spotLightComponent, uint32_t shadowIndex) :
	color(spotLightComponent.color),
	strength(spotLightComponent.strength),
	position(spotLightComponent.position),
	shininess(spotLightComponent.shininess),
	direction(glm::normalize(spotLightComponent.direction)),
	range(spotLightComponent.range),
	angles(spotLightComponent.angles),
	boundingSphereOrigin(spotLightComponent.boundingSphereOrigin),
	boundingSphereRadius(spotLightComponent.boundingSphereRadius),
	aabbExtents(spotLightComponent.aabbExtents),
	aabbOrigin(spotLightComponent.aabbOrigin),
	bitflag(0),
	shadowIndex(shadowIndex)
{
	bitflag |= (spotLightComponent.useShadow ? 1u : 0u) << 0; // Bit 0 = Simulate Shadow?
}

SpotLightShadowGPU::SpotLightShadowGPU(const SpotLightShadowComponent& shadowComponent) :
	nearPlane(shadowComponent.nearPlane),
	farPlane(shadowComponent.farPlane),
	shadowMapSize(glm::vec2(static_cast<float>(shadowComponent.textureSize))),
	viewProj(shadowComponent.viewProj)
{
}
