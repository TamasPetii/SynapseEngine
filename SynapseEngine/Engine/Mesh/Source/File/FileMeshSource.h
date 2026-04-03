#pragma once
#include "Engine/SynApi.h"
#include "../IMeshSource.h"
#include <optional>
#include <filesystem>

namespace Syn
{
	struct RawModel;
	class IMeshLoader;

	class SYN_API FileMeshSource : public IMeshSource
	{
	public:
		FileMeshSource(const std::filesystem::path& path, IMeshLoader* loader);
		virtual std::optional<RawModel> Produce() override;
	private:
		IMeshLoader* _loader;
		std::filesystem::path _path;
	};
}