#pragma once
#include "Engine/SynApi.h"
#include <string>
#include <vector>
#include <array>

#include "CookedMesh.h"
#include "CookedModel.h"
#include "Engine/Mesh/Data/Raw/RawModel.h"

namespace Syn
{
	struct SYN_API CookedUtils
	{
		static CookedMesh ConvertRawMesh(RawMesh&& rawMesh);
		static CookedModel ConvertRawModel(RawModel&& rawModel);
	};
}


