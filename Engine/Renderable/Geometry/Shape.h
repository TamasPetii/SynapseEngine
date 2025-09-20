#pragma once
#include <glm/gtc/constants.hpp>
#include "Engine/Renderable/Renderable.h"
#include "Engine/Renderable/Instanceable.h"
#include "Engine/Renderable/BoundingVolume.h"

class ENGINE_API Shape : public Renderable, public Instanceable, public BoundingVolume
{
public:
	void SetDescriptorArrayIndex(uint32_t descriptorArrayIndex) { this->descriptorArrayIndex = descriptorArrayIndex; }
	const auto& GetDescriptorArrayIndex() { return descriptorArrayIndex; }
protected:
	void Initialize();
	virtual void GenerateVertices() = 0;
	virtual void GenerateIndices() = 0;
	void GenerateTangents();
private:
	uint32_t descriptorArrayIndex = 0;
};

