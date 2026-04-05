#pragma once
#include "Engine/SynApi.h"
#include "BaseComponent/Component.h"
#include "Engine/Mesh/Data/StaticMesh.h"

namespace Syn
{
	struct SYN_API ModelComponent : public Component
	{
		ModelComponent();

		bool castShadow;
		bool receiveShadow;
		bool hasDirectxNormals;
		uint32_t modelIndex;
		uint32_t materialOffset = UINT32_MAX;
	};

	struct SYN_API ModelComponentGPU
	{
		ModelComponentGPU(uint32_t entityIndex, const ModelComponent& component);

		uint32_t entityIndex;
		uint32_t modelIndex;
		uint32_t flags;
		uint32_t materialOffset;
	};

	struct SYN_API VisibleModelData {
		uint32_t entityId;
		uint32_t modelIndex;
	};
}



