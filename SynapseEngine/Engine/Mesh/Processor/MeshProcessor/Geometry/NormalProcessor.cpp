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

#include "NormalProcessor.h"
#include <glm/glm.hpp>
#include "Engine/ServiceLocator.h"

#include <chrono>
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

namespace Syn
{
    void NormalProcessor::Process(CookedModel& cookedModel)
    {
        tf::Taskflow taskflow;
        tf::GuidedPartitioner partitioner(1);

        taskflow.for_each(cookedModel.meshes.begin(), cookedModel.meshes.end(), 
            [](CookedMesh& mesh) {
            if (mesh.hasNormals)
                return;

            if (mesh.lods.empty() || mesh.lods[0].indices.empty())
                return;

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
            },
            partitioner
        );

        ServiceLocator::Get<tf::Executor>()->run(taskflow).wait();
    }
}