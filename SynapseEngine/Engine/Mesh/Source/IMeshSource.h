#pragma once
#include "Engine/SynApi.h"
#include <optional>

namespace Syn 
{
	struct RawModel;

	class SYN_API IMeshSource
	{
	public:	
		virtual std::optional<RawModel> Produce() = 0;
	};
}



