// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "TransformComponent.h"

namespace Syn
{
	TransformComponent::TransformComponent() :
		translation(glm::vec3(0.f)),
		rotation(glm::vec3(0.f)),
		scale(glm::vec3(1.f)),
		transform(glm::mat4(1.f)),
		transformIT(glm::mat4(1.f))
	{}

	TransformComponentGPU::TransformComponentGPU(const TransformComponent& component) :
		transform(component.transform),
		transformIT(component.transformIT)
	{}

	TransformModelLinkGPU::TransformModelLinkGPU(uint32_t entityIndex, uint32_t modelDenseIndex) :
		entityIndex(entityIndex),
		modelDenseIndex(modelDenseIndex)
	{
	}
}