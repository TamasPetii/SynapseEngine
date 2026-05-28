#include "CapsuleColliderComponent.h"

namespace Syn
{
	CapsuleColliderComponent::CapsuleColliderComponent()
		: radius(0.5f), halfHeight(0.5f), localOffset(0.0f)
	{};

	CapsuleColliderComponentGPU::CapsuleColliderComponentGPU(const CapsuleColliderComponent& component, uint32_t entityIndex)
		: radius(component.radius),
		halfHeight(component.halfHeight),
		localOffset(component.localOffset),
		entityIndex(entityIndex)
	{}
}