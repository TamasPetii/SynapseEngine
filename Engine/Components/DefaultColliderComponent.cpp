#include "DefaultColliderComponent.h"

DefaultColliderGPU::DefaultColliderGPU(const DefaultColliderComponent& component, uint32_t objectIndex, bool isModel) : 
	aabbOrigin(component.aabbOrigin),
	objectIndex(objectIndex),
	aabbExtents(component.aabbExtents),
	bitflag(0),
	sphereOrigin(component.origin),
	sphereRadius(component.radius),
	projectedMinMax(component.projectedMinMax),
	linearDepth(component.linearDepth)
{
	bitflag |= (isModel ? 1u : 0u) << 0; // Bit 0 = Model or Shape?
	//This might be problematic for animation in future???
}
