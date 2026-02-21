#pragma once
#include "Engine/SynApi.h"
#include "../IMeshSource.h"
#include <optional>
#include <string>

namespace Syn
{
	struct RawModel;

	class SYN_API ProceduralMeshSource : public IMeshSource
	{
	public:
		ProceduralMeshSource(std::string name) : _name(std::move(name)) {}
	protected:
		std::string _name;
	};
}