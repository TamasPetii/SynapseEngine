#include "TangentProcessor.h"
#include <glm/glm.hpp>
#include "Engine/ServiceLocator.h"
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

namespace Syn
{
    void TangentProcessor::Process(CookedModel& cookedModel)
    {
        tf::Taskflow taskflow;
        tf::GuidedPartitioner partitioner(1);

        taskflow.for_each(cookedModel.meshes.begin(), cookedModel.meshes.end(),
            [](CookedMesh& mesh) {
            if (mesh.hasTangents || !mesh.hasNormals)
                return;

            if (mesh.lods.empty() || mesh.lods[0].indices.empty())
                return;

            for (auto& vertex : mesh.vertices)
                vertex.tangent = glm::vec3(0.0f);

            const auto& indices = mesh.lods[0].indices;
            for (size_t i = 0; i < indices.size(); i += 3)
            {
                uint32_t i0 = indices[i];
                uint32_t i1 = indices[i + 1];
                uint32_t i2 = indices[i + 2];

                const Vertex& v0 = mesh.vertices[i0];
                const Vertex& v1 = mesh.vertices[i1];
                const Vertex& v2 = mesh.vertices[i2];

                glm::vec3 edge1 = v1.position - v0.position;
                glm::vec3 edge2 = v2.position - v0.position;

                glm::vec2 deltaUV1 = v1.uv - v0.uv;
                glm::vec2 deltaUV2 = v2.uv - v0.uv;

                float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

                if (std::isinf(f) || std::isnan(f))
                    continue;

                glm::vec3 tangent;
                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

                mesh.vertices[i0].tangent += tangent;
                mesh.vertices[i1].tangent += tangent;
                mesh.vertices[i2].tangent += tangent;
            }

            // Gram-Schmidt
            for (auto& vertex : mesh.vertices)
            {
                glm::vec3 n = vertex.normal;
                glm::vec3 t = vertex.tangent;

                if (glm::length(t) < 0.00001f)
                {
                    glm::vec3 c1 = glm::cross(n, glm::vec3(0.0f, 0.0f, 1.0f));
                    glm::vec3 c2 = glm::cross(n, glm::vec3(0.0f, 1.0f, 0.0f));
                    t = glm::length(c1) > glm::length(c2) ? c1 : c2;
                }

                // Orthogonalize: T = normalize(T - N * dot(N, T))
                vertex.tangent = glm::normalize(t - n * glm::dot(n, t));
            }

            mesh.hasTangents = true;
            },
            partitioner
        );

        ServiceLocator::GetTaskExecutor()->run(taskflow).wait();
    }
}