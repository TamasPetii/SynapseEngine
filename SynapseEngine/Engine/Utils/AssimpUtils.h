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
#include "Engine/SynApi.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/common.hpp>
#include <assimp/types.h>
#include <assimp/postprocess.h>

namespace Syn
{
	class SYN_API AssimpUtils
	{
	public:
		static glm::vec3 ConvertAssimpToGlm(const aiVector3D& vector);
		static glm::vec3 ConvertAssimpToGlm(const aiColor3D& vector);
		static glm::quat ConvertAssimpToGlm(const aiQuaternion& vector);
		static glm::mat4 ConvertAssimpToGlm(const aiMatrix4x4& matrix);

		static constexpr aiPostProcessSteps ImportFlags = static_cast<aiPostProcessSteps>(
			aiProcess_Triangulate | 
			aiProcess_JoinIdenticalVertices | 
			aiProcess_CalcTangentSpace | 
			aiProcess_LimitBoneWeights);
	};
}