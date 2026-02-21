#include "ShapeMeshSource.h"
#include "Engine/Mesh/Intermediate/RawModel.h"
#include <map>
#include <tuple>

namespace Syn
{
    //Template Method Pattern
    std::optional<RawModel> ShapeMeshSource::Produce()
    {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;

        std::vector<uint32_t> indices;

        GeneratePositions(positions);
        GenerateIndices(indices);

        uvs.resize(positions.size(), glm::vec2(0.0f));
        normals.resize(positions.size(), glm::vec3(0.0f, 1.0f, 0.0f));

        GenerateUVs(uvs);
        GenerateNormals(positions, indices, normals);

        RawModel model{};
        RawMesh mesh{};
        mesh.name = _name;
        mesh.materialIndex = 0;
        mesh.indices = std::move(indices);

        mesh.vertices.reserve(positions.size());
        for (size_t i = 0; i < positions.size(); ++i)
        {
            RawVertex v{
				v.position = positions[i],
				v.normal = normals[i],
				v.tangent = glm::vec3(0.0f),
				v.uv = uvs[i],

            };

            mesh.vertices.push_back(v);
        }

        model.meshes.push_back(std::move(mesh));

        RawNode rootNode{};
        rootNode.localTransform = glm::mat4(1.0f);
        rootNode.globalTransform = glm::mat4(1.0f);
        rootNode.globalTransformIT = glm::mat4(1.0f);
        model.nodeTransforms.push_back(rootNode);

        RawMeshNodeDescriptor desc{};
        desc.meshIndex = 0;
        desc.nodeIndex = 0;
        desc.parentNodeIndex = UINT16_MAX;
        desc.vertexCount = static_cast<uint32_t>(model.meshes[0].vertices.size());
        desc.indexCount = static_cast<uint32_t>(model.meshes[0].indices.size());
        model.meshNodeDescriptors.push_back(desc);

        return model;
    }

    void ShapeMeshSource::GenerateNormals(const std::vector<glm::vec3>& positions, const std::vector<uint32_t>& indices, std::vector<glm::vec3>& outNormals)
    {
        auto getQuantizedPos = [](const glm::vec3& p) -> std::tuple<int, int, int> {
            constexpr float precision = 10000.0f;
            return {
                static_cast<int>(std::round(p.x * precision)),
                static_cast<int>(std::round(p.y * precision)),
                static_cast<int>(std::round(p.z * precision))
            };
            };

        std::map<std::tuple<int, int, int>, glm::vec3> positionNormals;

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            uint32_t i0 = indices[i];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            glm::vec3 edge1 = positions[i1] - positions[i0];
            glm::vec3 edge2 = positions[i2] - positions[i0];

            glm::vec3 faceNormal = glm::cross(edge1, edge2);

            positionNormals[getQuantizedPos(positions[i0])] += faceNormal;
            positionNormals[getQuantizedPos(positions[i1])] += faceNormal;
            positionNormals[getQuantizedPos(positions[i2])] += faceNormal;
        }

        for (size_t i = 0; i < positions.size(); ++i)
        {
            glm::vec3 accumulatedNormal = positionNormals[getQuantizedPos(positions[i])];

            if (glm::length(accumulatedNormal) > 0.0001f)
                outNormals[i] = glm::normalize(accumulatedNormal);
            else
                outNormals[i] = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }
}