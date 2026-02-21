#pragma once
#include "Engine/SynApi.h"
#include <optional>
#include <filesystem>

#include "Engine/Mesh/Data/Raw/RawModel.h"

namespace Syn 
{
	class SYN_API IMeshLoader
	{
	public:
		virtual std::optional<RawModel> LoadFile(const std::filesystem::path& path) = 0;
		virtual std::vector<std::string> GetSupportedExtensions() const = 0;
	};
}

