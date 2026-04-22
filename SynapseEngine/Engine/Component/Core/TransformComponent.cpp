#include "TransformComponent.h"

namespace Syn
{
	TransformComponent::TransformComponent() :
		translation(glm::vec3(0.f)),
		rotation(glm::vec3(0.f)),
		scale(glm::vec3(1.f)),
		transform(glm::mat4(1.f)),
		transformIT(glm::mat4(1.f))
	{}

	TransformComponentGPU::TransformComponentGPU(const TransformComponent& component) :
		transform(component.transform),
		transformIT(component.transformIT)
	{}
}