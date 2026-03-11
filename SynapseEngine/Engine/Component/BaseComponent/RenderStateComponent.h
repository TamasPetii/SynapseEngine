#pragma once
#include "Engine/SynApi.h"
#include "Component.h"
#include <glm/glm.hpp>

namespace Syn
{
	struct SYN_API RenderStateComponent : public Component
	{
		bool isVisible = false;
	};
}