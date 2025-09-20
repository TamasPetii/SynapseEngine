#pragma once
#include "BaseComponents/Component.h"
#include "Engine/EngineApi.h"
#include <stdint.h>

struct ENGINE_API ModelRenderIndicesGPU
{
	uint32_t entityIndex;
	uint32_t transformIndex;
	uint32_t modelIndex; //Defines: VertexBuffer, MaterialBuffer, NodeTransformBuffer
	uint32_t animationIndex; //Defines: BoneBuffer
	uint32_t animationTransformIndex; //Defines: Unique NodeTransformBuffer
	uint32_t bitflag;
};

struct ENGINE_API ShapeRenderIndicesGPU
{
	uint32_t entityIndex;
	uint32_t transformIndex;
	uint32_t shapeIndex;
	uint32_t materialIndex;
	uint32_t bitflag;
};