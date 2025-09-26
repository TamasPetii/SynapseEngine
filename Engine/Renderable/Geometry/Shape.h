#pragma once
#include <glm/gtc/constants.hpp>
#include "Engine/Renderable/Renderable.h"
#include "Engine/Renderable/Instanceable.h"
#include "Engine/Renderable/BoundingVolume.h"
#include "Engine/Utils/BufferArrayIndexed.h"

class ENGINE_API Shape : public Renderable, public Instanceable, public BoundingVolume, public BufferArrayIndexed
{
public:
	Shape() = default;
protected:
	void Initialize();
	virtual void GenerateVertices() = 0;
	virtual void GenerateIndices() = 0;
	void GenerateTangents();
};

