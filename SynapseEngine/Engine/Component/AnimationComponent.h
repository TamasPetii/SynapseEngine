#pragma once
#include "Engine/SynApi.h"
#include "BaseComponent/Component.h"
#include "Engine/Animation/Data/Animation.h"

namespace Syn
{
	struct SYN_API AnimationComponent : public Component
	{
		AnimationComponent();

		bool isReady;
		float time;
		float speed;
		uint32_t frameIndex;
		uint32_t animationIndex;
	};

	struct SYN_API AnimationComponentGPU
	{
		AnimationComponentGPU(const AnimationComponent& component);

		uint32_t animationIndex;
		uint32_t frameIndex;
		uint32_t padding0;
		uint32_t padding1;
	};
}



