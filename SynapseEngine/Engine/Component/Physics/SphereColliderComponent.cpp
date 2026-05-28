#include "SphereColliderComponent.h"

namespace Syn
{
	SphereColliderComponent::SphereColliderComponent() :
		radius(1.0f),
		localOffset(glm::vec3(0.f))
	{}

	SphereColliderComponentGPU::SphereColliderComponentGPU(const SphereColliderComponent& component, uint32_t entityIndex) :
		radius(component.radius),
		localOffset(component.localOffset),
		entityIndex(entityIndex)
	{}
}