#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Data/Raw/RawModel.h"
#include <filesystem>

namespace Syn 
{
	class MeshUtils
	{
	public:
		static void LogRawModel(const std::filesystem::path& path, const RawModel& model);
	};
}

