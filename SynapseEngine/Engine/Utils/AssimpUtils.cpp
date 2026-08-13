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

#include "AssimpUtils.h"

namespace Syn
{

	glm::vec3 AssimpUtils::ConvertAssimpToGlm(const aiVector3D& vector)
	{
		return glm::vec3(vector.x, vector.y, vector.z);
	}

	glm::vec3 AssimpUtils::ConvertAssimpToGlm(const aiColor3D& vector)
	{
		return glm::vec3(vector.r, vector.g, vector.b);
	}

	glm::quat AssimpUtils::ConvertAssimpToGlm(const aiQuaternion& vector)
	{
		return glm::quat(vector.w, vector.x, vector.y, vector.z);
	}

	glm::mat4 AssimpUtils::ConvertAssimpToGlm(const aiMatrix4x4& matrix)
	{
		glm::mat4 result;

		result[0][0] = matrix.a1;
		result[1][0] = matrix.a2;
		result[2][0] = matrix.a3;
		result[3][0] = matrix.a4;

		result[0][1] = matrix.b1;
		result[1][1] = matrix.b2;
		result[2][1] = matrix.b3;
		result[3][1] = matrix.b4;

		result[0][2] = matrix.c1;
		result[1][2] = matrix.c2;
		result[2][2] = matrix.c3;
		result[3][2] = matrix.c4;

		result[0][3] = matrix.d1;
		result[1][3] = matrix.d2;
		result[2][3] = matrix.d3;
		result[3][3] = matrix.d4;

		return result;
	}
}