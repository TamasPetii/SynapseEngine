#pragma once
#include "Engine/SynApi.h"
#include "RawMesh.h"
#include "../Common/MeshInstanceDescriptor.h"
#include "../Common/TransformNode.h"
#include <string>

namespace Syn
{
	struct SYN_API RawModel
	{
		std::vector<RawMesh> meshes;
		std::vector<std::string> materialNames;
		std::vector<TransformNode> nodeTransforms;
		std::vector<MeshInstanceDescriptor> meshNodeDescriptors;
	};
}

