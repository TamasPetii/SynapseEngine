#include "BoxColliderComponent.h"

namespace Syn
{
	BoxColliderComponent::BoxColliderComponent() :
		halfExtents(glm::vec3(1.0f)),
		localOffset(glm::vec3(0.f))
	{}

	BoxColliderComponentGPU::BoxColliderComponentGPU(const BoxColliderComponent& component, uint32_t entityIndex) :
		halfExtents(component.halfExtents),
		localOffset(component.localOffset),
		entityIndex(entityIndex)
	{}
}