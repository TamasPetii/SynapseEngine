#pragma once
#include "Engine/SynApi.h"
#include "BaseComponent/Component.h"
#include <string>

namespace Syn
{
	struct SYN_API TagComponent : public Component
	{
		TagComponent();
		TagComponent(const std::string& name, const std::string& tag);

		std::string name;
		std::string tag;
	};
}