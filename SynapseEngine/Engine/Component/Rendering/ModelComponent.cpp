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

#include "ModelComponent.h"

namespace Syn
{
	ModelComponent::ModelComponent() :
		castShadow(true),
		receiveShadow(true),
		modelIndex(UINT32_MAX)
	{}

	ModelComponentGPU::ModelComponentGPU(uint32_t entityIndex, const ModelComponent& component) :
		entityIndex(entityIndex),
		modelIndex(component.modelIndex),
		materialOffset(component.materialOffset),
		pipelineOffset(component.pipelineOffset)
	{
		uint32_t flags = 0;
		if (component.castShadow)        flags |= (1 << 0);
		if (component.receiveShadow)     flags |= (1 << 1);
		this->flags = flags;
	}
}