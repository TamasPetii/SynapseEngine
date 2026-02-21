#pragma once
#include "Engine/SynApi.h"
#include "Engine/Mesh/Intermediate/RawModel.h"
#include <optional>
#include <filesystem>

namespace Syn 
{
	class SYN_API IMeshLoader
	{
	public:
		virtual std::optional<RawModel> LoadFile(const std::filesystem::path& path) = 0;
		virtual std::vector<std::string> GetSupportedExtensions() const = 0;
	};
}

