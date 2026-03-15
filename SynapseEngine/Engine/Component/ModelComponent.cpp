#include "ModelComponent.h"
#include "Engine/Registry/Entity.h"

namespace Syn
{
	ModelComponent::ModelComponent() :
		castShadow(true),
		receiveShadow(true),
		hasDirectxNormals(false),
		modelIndex(NULL_INDEX)
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