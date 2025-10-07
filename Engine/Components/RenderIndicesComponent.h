#pragma once
#include "BaseComponents/Component.h"
#include "Engine/EngineApi.h"
#include <stdint.h>

struct ENGINE_API RenderIndicesComponent : public Component
{
	uint32_t transformIndex;
	uint32_t assetIndex; //Unique model or shape index
	uint32_t objectIndex; //Model component or shape component index
	uint32_t bitflag; //Pack shape/model here!
};

struct ENGINE_API RenderIndicesGPU
{
	RenderIndicesGPU(const RenderIndicesComponent& component, uint32_t entity);

	uint32_t entityIndex = UINT32_MAX;
	uint32_t transformIndex = UINT32_MAX;
	uint32_t assetIndex = UINT32_MAX; //Unique model or shape index
	uint32_t objectIndex = UINT32_MAX; //Model component or shape component index
	uint32_t bitflag = 0;
};

//These are GPU packed too!
struct ENGINE_API ModelRenderIndices : public Component
{
	uint32_t animationIndex; //Defines: BoneBuffer
	uint32_t animationTransformIndex; //Defines: Unique NodeTransformBuffer
};

//These are GPU packed too!
struct ENGINE_API ShapeRenderIndices : public Component
{
	uint32_t materialIndex;
};


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