#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/Raw/RawModel.h"
#include <filesystem>

#include "Engine/Mesh/Data/Cooked/CookedMesh.h"
#include "Engine/Mesh/Data/Cooked/CookedModel.h"
#include "Engine/Mesh/Data/Raw/RawModel.h"

namespace Syn 
{
	class MeshUtils
	{
	public:
		static void LogRawModel(const std::filesystem::path& path, const RawModel& model);
	};
}

