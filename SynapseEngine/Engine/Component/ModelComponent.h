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
		ModelComponentGPU(uint32_t entityIndex, const ModelComponent& component);

		uint32_t entityIndex;
		uint32_t modelIndex;
		uint32_t flags;
		uint32_t pad0;
	};

	struct SYN_API VisibleModelData {
		uint32_t entityId;
		uint32_t modelIndex;
		uint32_t packedFlags;
		uint32_t padding;
	};
}



