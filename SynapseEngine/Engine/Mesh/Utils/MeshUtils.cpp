#include "MeshUtils.h"
#include "Engine/Logger/Logger.h"
#include <sstream>
#include <format>

namespace Syn
{
    void MeshUtils::LogRawModel(const std::filesystem::path& path, const RawModel& model)
    {
        std::stringstream ss;

        ss << "\n==================================================\n";
        ss << std::format("  MODEL LOADED: {}\n", path.string());
        ss << "==================================================\n";

        ss << std::format("[Summary] Meshes: {} | Nodes: {} | Materials: {} | Descriptors: {}\n",
            model.meshes.size(), model.nodeTransforms.size(),
            model.materialNames.size(), model.meshNodeDescriptors.size());

        ss << "--- Materials ---\n";
        for (size_t i = 0; i < model.materialNames.size(); ++i)
        {
            ss << std::format("  [{}] Name: '{}'\n", i, model.materialNames[i]);
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
