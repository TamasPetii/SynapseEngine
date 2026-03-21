#pragma once
#include "Engine/SynApi.h"
#include <optional>

#include "Engine/Animation/Data/Raw/RawAnimation.h"

namespace Syn
{
	class SYN_API IAnimationSource
	{
	public:
		virtual std::optional<RawAnimation> Produce() = 0;
	};
}



