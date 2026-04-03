#pragma once
#include "Engine/SynApi.h"
#include "../ProceduralMeshSource.h"
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>
#include <span>

namespace Syn
{
	class SYN_API ShapeMeshSource : public ProceduralMeshSource
	{
	public:
		ShapeMeshSource(std::string name) : ProceduralMeshSource(name) {}
		virtual ~ShapeMeshSource() = default;
		virtual std::optional<RawModel> Produce() override;
	protected:
		virtual void GeneratePositions(std::vector<glm::vec3>& outPositions) = 0;
		virtual void GenerateIndices(std::vector<uint32_t>& outIndices) = 0;
		virtual void GenerateUVs(std::span<glm::vec2> outUVs) = 0;
		virtual void GenerateNormals(std::span<const glm::vec3> positions, std::span<const uint32_t> indices, std::span<glm::vec3> outNormals);
	};
}