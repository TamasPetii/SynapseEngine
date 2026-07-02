#pragma once
#include "Engine/SynApi.h"
#include "Engine/Component/Core/Component.h"
#include <string>

namespace Syn
{
	struct SYN_API TagComponent : public Component
	{
		TagComponent();
		TagComponent(const std::string& name, const std::string& tag);

		std::string name;
		std::string tag;

		bool localEnabled = true;
		bool globalEnabled = true;
	};
}