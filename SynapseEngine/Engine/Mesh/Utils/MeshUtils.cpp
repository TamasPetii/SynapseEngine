#include "MeshUtils.h"
#include "Engine/Logger/Logger.h"
#include <sstream>
#include <format>

namespace Syn
{
    GpuMeshCollider MeshUtils::TransformCollider(const GpuMeshCollider& local, const glm::mat4& transform)
    {
        glm::mat3 rotMatrix = glm::mat3(transform);
        glm::mat3 absTransform(
            glm::abs(rotMatrix[0]),
            glm::abs(rotMatrix[1]),
            glm::abs(rotMatrix[2])
        );

        float scaleX = glm::length(rotMatrix[0]);
        float scaleY = glm::length(rotMatrix[1]);
        float scaleZ = glm::length(rotMatrix[2]);
        float maxScale = std::max({ scaleX, scaleY, scaleZ });

        // Sphere transform
        glm::vec3 worldCenter = glm::vec3(transform * glm::vec4(local.center, 1.0f));
        float worldRadius = local.radius * maxScale;

        //AABB transform
        glm::vec3 localAabbCenter = (local.aabbMax + local.aabbMin) * 0.5f;
        glm::vec3 localAabbExtents = (local.aabbMax - local.aabbMin) * 0.5f;

        glm::vec3 worldAabbCenter = glm::vec3(transform * glm::vec4(localAabbCenter, 1.0f));
        glm::vec3 worldAabbExtents = absTransform * localAabbExtents;

        GpuMeshCollider world;
        world.center = worldCenter;
        world.radius = worldRadius;
        world.aabbMin = worldAabbCenter - worldAabbExtents;
        world.aabbMax = worldAabbCenter + worldAabbExtents;

        return world;
    }

    GpuMeshletCollider MeshUtils::TransformCollider(const GpuMeshletCollider& local, const glm::mat4& transform, const glm::mat4& transformIT)
    {
        GpuMeshCollider baseLocal;
        baseLocal.center = local.center;
        baseLocal.radius = local.radius;
        baseLocal.aabbMin = local.aabbMin;
        baseLocal.aabbMax = local.aabbMax;

        GpuMeshCollider baseWorld = TransformCollider(baseLocal, transform);

        GpuMeshletCollider world;
        world.center = baseWorld.center;
        world.radius = baseWorld.radius;
        world.aabbMin = baseWorld.aabbMin;
        world.aabbMax = baseWorld.aabbMax;

        //Cone transform
        world.apex = glm::vec3(transform * glm::vec4(local.apex, 1.0f));
        world.axis = glm::normalize(glm::vec3(transformIT * glm::vec4(local.axis, 0.0f)));
        world.cutoff = local.cutoff;

        return world;
    }

    void MeshUtils::LogRawModel(const std::filesystem::path& path, const RawModel& model)
    {
        std::stringstream ss;

        ss << "\n==================================================\n";
        ss << std::format("  MODEL LOADED: {}\n", path.string());
        ss << "==================================================\n";

        ss << std::format("[Summary] Meshes: {} | Nodes: {} | Materials: {} | Descriptors: {}\n",
            model.meshes.size(), model.nodeTransforms.size(),
            model.materials.size(), model.meshNodeDescriptors.size());

        ss << "--- Materials ---\n";
        for (size_t i = 0; i < model.materials.size(); ++i)
        {
            ss << std::format("  [{}] Name: '{}'\n", i, model.materials[i].name);
        }

        ss << "--- Meshes (Flat Layout) ---\n";
        for (size_t i = 0; i < model.meshes.size(); ++i)
        {
            const auto& mesh = model.meshes[i];
            ss << std::format("  [Mesh {}] Name: '{}' | MatIdx: {} | Verts: {} | Indices: {}\n",
                i, mesh.name, mesh.materialIndex, mesh.vertices.size(), mesh.indices.size());
        }

        ss << "--- Nodes & Descriptors (Flat Hierarchy) ---\n";
        for (size_t i = 0; i < model.nodeTransforms.size(); ++i)
        {
            std::string attachedMeshes = "";
            uint16_t parentIndex = UINT16_MAX;

            for (const auto& desc : model.meshNodeDescriptors)
            {
                if (desc.nodeIndex == i)
                {
                    attachedMeshes += std::to_string(desc.meshIndex) + " ";
                    parentIndex = desc.parentNodeIndex;
                }
            }

            std::string parentStr = (parentIndex == UINT16_MAX) ? "ROOT" : std::to_string(parentIndex);

            if (attachedMeshes.empty()) {
                ss << std::format("  [Node {}] Parent: {} | Attached Meshes: NONE (Transform only)\n", i, parentStr);
            }
            else {
                if (!attachedMeshes.empty()) attachedMeshes.pop_back();
                ss << std::format("  [Node {}] Parent: {} | Attached Meshes: [{}]\n", i, parentStr, attachedMeshes);
            }
        }
        ss << "==================================================";

        Info("{}", ss.str());
    }
}
