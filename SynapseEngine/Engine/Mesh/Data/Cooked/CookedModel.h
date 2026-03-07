#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <vector>
#include <array>

#include "CookedMesh.h"
#include "../Common/TransformNode.h"
#include "../Common/MeshInstanceDescriptor.h"
#include "../Common/MaterialInfo.h"

namespace Syn
{
	struct SYN_API CookedModel
	{
		CookedMeshCollisionData globalCollider;
		std::vector<CookedMesh> meshes;
		std::vector<MaterialInfo> materials;
		std::vector<TransformNode> nodeTransforms;
		std::vector<MeshInstanceDescriptor> meshNodeDescriptors;
	};
}


