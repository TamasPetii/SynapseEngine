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
		ModelComponentGPU(const ModelComponent& component);

		uint32_t modelIndex;
		uint32_t flags;
		uint32_t pad0;
		uint32_t pad1;
	};
}



