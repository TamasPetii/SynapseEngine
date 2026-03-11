#include "ModelComponent.h"

namespace Syn
{
	ModelComponent::ModelComponent() :
		castShadow(true),
		receiveShadow(true),
		hasDirectxNormals(false),
		model(nullptr)
	{}
}