#pragma once
#include "Engine/SynApi.h"
#include "../IAnimationSource.h"
#include <optional>
#include <string>

namespace Syn
{
	class SYN_API ProceduralAnimationSource : public IAnimationSource
	{
	public:
		ProceduralAnimationSource(std::string name) : _name(std::move(name)) {}
	protected:
		std::string _name;
	};
}