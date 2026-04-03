#pragma once
#include "Engine/SynApi.h"
#include "../IAnimationSource.h"
#include <optional>
#include <filesystem>

namespace Syn
{
	struct RawAnimation;
	class IAnimationLoader;

	class SYN_API FileAnimationSource : public IAnimationSource
	{
	public:
		FileAnimationSource(const std::filesystem::path& path, IAnimationLoader* loader);
		virtual std::optional<RawAnimation> Produce() override;
	private:
		IAnimationLoader* _loader;
		std::filesystem::path _path;
	};
}