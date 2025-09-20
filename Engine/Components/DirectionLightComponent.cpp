#include "DirectionLightComponent.h"

DirectionLightShadow::DirectionLightShadow()
{
}

DirectionLightComponent::DirectionLightComponent() : 
	direction(defaultDirectionLightDirection)
{
	strength = 5.f;
}


DirectionLightGPU::DirectionLightGPU(const DirectionLightComponent& directionLightComponent) : 
	color(directionLightComponent.color),
	strength(directionLightComponent.strength),
	direction(glm::normalize(directionLightComponent.direction)),
	shininess(directionLightComponent.shininess)
{
}
