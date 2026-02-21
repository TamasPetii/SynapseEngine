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
			aiProcess_CalcTangentSpace);
	};
}