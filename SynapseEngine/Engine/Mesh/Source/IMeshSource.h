#pragma once
#include "Engine/SynApi.h"
#include <optional>

#include "Engine/Mesh/Data/Raw/RawModel.h"

namespace Syn 
{
	class SYN_API IMeshSource
	{
	public:	
		virtual std::optional<RawModel> Produce() = 0;
	};
}



