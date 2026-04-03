#include "ModelComponent.h"

namespace Syn
{
	ModelComponent::ModelComponent() :
		castShadow(true),
		receiveShadow(true),
		hasDirectxNormals(false),
		modelIndex(UINT32_MAX)
	{}

	ModelComponentGPU::ModelComponentGPU(uint32_t entityIndex, const ModelComponent& component) :
		entityIndex(entityIndex),
		modelIndex(component.modelIndex),
		materialOffset(component.materialOffset)
	{
		uint32_t flags = 0;
		if (component.castShadow)        flags |= (1 << 0);
		if (component.receiveShadow)     flags |= (1 << 1);
		if (component.hasDirectxNormals) flags |= (1 << 2);
		this->flags = flags;
	}
}