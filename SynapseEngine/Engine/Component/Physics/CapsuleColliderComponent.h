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

#pragma once
#include "Engine/Component/Core/Component.h"
#include <glm/glm.hpp>

namespace Syn
{
    struct SYN_API CapsuleColliderComponent : public Component
    {
		CapsuleColliderComponent();

		float radius;
		float halfHeight;
		glm::vec3 localOffset;
    };

	struct SYN_API CapsuleColliderComponentGPU
	{
		CapsuleColliderComponentGPU(const CapsuleColliderComponent& component, uint32_t entityIndex);

		glm::vec3 localOffset;
		float radius;
		float halfHeight;
		uint32_t entityIndex;
		float _pad1;
		float _pad2;
	};
}