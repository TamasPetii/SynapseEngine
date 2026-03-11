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

	ModelComponentGPU::ModelComponentGPU(const ModelComponent& component) :
		modelIndex(component.modelIndex),
		pad0(0),
		pad1(0)
	{
		uint32_t flags = 0;
		if (component.castShadow)        flags |= (1 << 0);
		if (component.receiveShadow)     flags |= (1 << 1);
		if (component.hasDirectxNormals) flags |= (1 << 2);
		this->flags = flags;
	}
}