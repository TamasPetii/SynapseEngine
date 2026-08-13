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

#include "FrustumCollider.h"

namespace Syn
{
    void FrustumCollider::Update(const glm::mat4& mat)
    {
        // Gribb-Hartmann Algorithm
        // 0: Near, 1: Right, 2: Left, 3: Top, 4: Bottom, 5: Far

        // 0. Near plane (for Vulkan [0, 1] depth): z >= 0
        planes[0] = NormalizePlane(glm::vec4(
            mat[0][2], mat[1][2], mat[2][2], mat[3][2]
        ));

        // 1. Right plane: w - x >= 0
        planes[1] = NormalizePlane(glm::vec4(
            mat[0][3] - mat[0][0], mat[1][3] - mat[1][0], mat[2][3] - mat[2][0], mat[3][3] - mat[3][0]
        ));

        // 2. Left plane: w + x >= 0
        planes[2] = NormalizePlane(glm::vec4(
            mat[0][3] + mat[0][0], mat[1][3] + mat[1][0], mat[2][3] + mat[2][0], mat[3][3] + mat[3][0]
        ));

        // 3. Top plane: w - y >= 0
        planes[3] = NormalizePlane(glm::vec4(
            mat[0][3] - mat[0][1], mat[1][3] - mat[1][1], mat[2][3] - mat[2][1], mat[3][3] - mat[3][1]
        ));

        // 4. Bottom plane: w + y >= 0
        planes[4] = NormalizePlane(glm::vec4(
            mat[0][3] + mat[0][1], mat[1][3] + mat[1][1], mat[2][3] + mat[2][1], mat[3][3] + mat[3][1]
        ));

        // 5. Far plane: w - z >= 0
        planes[5] = NormalizePlane(glm::vec4(
            mat[0][3] - mat[0][2], mat[1][3] - mat[1][2], mat[2][3] - mat[2][2], mat[3][3] - mat[3][2]
        ));
    }
}