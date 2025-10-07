#include "RenderIndicesComponent.h"

RenderIndicesGPU::RenderIndicesGPU(const RenderIndicesComponent& component, uint32_t entity) : 
	entityIndex(entity),
	transformIndex(component.transformIndex),
	assetIndex(component.assetIndex),
	objectIndex(component.objectIndex),
	bitflag(component.bitflag)
{
}
