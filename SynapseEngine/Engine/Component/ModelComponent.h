#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/StaticMesh.h"
#include "BaseComponent/RenderStateComponent.h"

namespace Syn
{
	struct SYN_API ModelComponent : public RenderStateComponent
	{
		ModelComponent();

		bool castShadow;
		bool receiveShadow;
		bool hasDirectxNormals;
		uint32_t modelIndex;
	};

	struct SYN_API ModelComponentGPU
	{

	};
}



