#pragma once
#include "BaseComponents/Component.h"
#include "Engine/Utils/Material.h"

struct ENGINE_API MaterialComponent : public Component
{
	std::shared_ptr<Material> material = nullptr;
};