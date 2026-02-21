#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <vector>
#include <array>

#include "CookedMesh.h"
#include "../Common/TransformNode.h"
#include "../Common/MeshInstanceDescriptor.h"

namespace Syn
{
	struct SYN_API CookedModel
	{
		CookedMeshCollisionData globalCollider;
		std::vector<CookedMesh> meshes;
		std::vector<std::string> materialNames;
		std::vector<TransformNode> nodeTransforms;
		std::vector<MeshInstanceDescriptor> meshNodeDescriptors;
	};
}


