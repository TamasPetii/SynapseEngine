#include "NormalProcessor.h"
#include <glm/glm.hpp>

namespace Syn
{
    void NormalProcessor::Process(CookedModel& cookedModel)
    {
        for (auto& mesh : cookedModel.meshes)
        {
            if (mesh.hasNormals)
                continue;

            if (mesh.lods.empty() || mesh.lods[0].indices.empty())
                continue;

            for (auto& vertex : mesh.vertices)
                vertex.normal = glm::vec3(0.0f);

            const auto& indices = mesh.lods[0].indices;
            for (size_t i = 0; i < indices.size(); i += 3)
            {
                uint32_t i0 = indices[i];
                uint32_t i1 = indices[i + 1];
                uint32_t i2 = indices[i + 2];

                glm::vec3 p0 = mesh.vertices[i0].position;
                glm::vec3 p1 = mesh.vertices[i1].position;
                glm::vec3 p2 = mesh.vertices[i2].position;

                glm::vec3 edge1 = p1 - p0;
                glm::vec3 edge2 = p2 - p0;

                glm::vec3 faceNormal = glm::cross(edge1, edge2);

                mesh.vertices[i0].normal += faceNormal;
                mesh.vertices[i1].normal += faceNormal;
                mesh.vertices[i2].normal += faceNormal;
            }

            for (auto& vertex : mesh.vertices)
            {
                if (glm::length(vertex.normal) > 0.00001f)
                    vertex.normal = glm::normalize(vertex.normal);
                else
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            }

            mesh.hasNormals = true;
        }
    }
}