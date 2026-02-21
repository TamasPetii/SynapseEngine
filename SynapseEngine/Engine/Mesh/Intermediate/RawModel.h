#pragma once
#include "Engine/SynApi.h"
#include "RawMesh.h"
#include "RawNode.h"
#include "RawMeshNodeDescriptor.h"
#include <string>

namespace Syn
{
	struct SYN_API RawModel
	{
		std::vector<RawMesh> meshes;
		std::vector<RawNode> nodeTransforms;
		std::vector<std::string> materialNames;
		std::vector<RawMeshNodeDescriptor> meshNodeDescriptors;
	};
}

