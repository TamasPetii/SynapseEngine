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
}