#pragma once
#include "Engine/SynApi.h"
#include "../IImageSource.h"
#include <optional>
#include <string>

namespace Syn
{
	class SYN_API ProceduralImageSource : public IImageSource
	{
	public:
		ProceduralImageSource(std::string name) : _name(std::move(name)) {}
	protected:
		std::string _name;
	};
}