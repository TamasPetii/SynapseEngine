#include "AnimationComponent.h"

namespace Syn
{
	AnimationComponent::AnimationComponent() :
		isReady(false),
		time(0.0),
		speed(1.f),
		animationIndex(UINT32_MAX)
	{}

	AnimationComponentGPU::AnimationComponentGPU(const AnimationComponent& component) :

		animationIndex(component.animationIndex),
		frameIndex(component.frameIndex),
		padding0(0),
		padding1(0)
	{
	}
}