#include "SpotLightComponent.h"

SpotLightShadow::SpotLightShadow() : 
	viewProj(glm::mat4(1))
{
}

SpotLightComponent::SpotLightComponent() :
	position(glm::vec3(0.f)),
	transform(glm::mat4(1))
{
	strength = 10.f;
}

SpotLightGPU::SpotLightGPU(const SpotLightComponent& spotLightComponent) :
	color(spotLightComponent.color),
	strength(spotLightComponent.strength),
	position(spotLightComponent.position),
	shininess(spotLightComponent.shininess),
	direction(glm::normalize(spotLightComponent.direction)),
	length(spotLightComponent.length),
	angles(spotLightComponent.angles),
	bitflag(0)
{
	bitflag |= (spotLightComponent.shadow.use ? 1u : 0u) << 0; // Bit 0 = Simulate Shadow?
}
