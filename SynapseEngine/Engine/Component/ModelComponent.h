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
		std::shared_ptr<StaticMesh> model;
	};

	struct SYN_API ModelComponentGPU
	{

	};
}



