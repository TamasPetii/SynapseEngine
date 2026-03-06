#include "AssimpLoader.h"
#include "Engine/Utils/AssimpUtils.h"
#include "Engine/Mesh/Utils/MeshUtils.h"
#include <Assimp/Importer.hpp>
#include <Assimp/postprocess.h>
#include <Assimp/scene.h>
#include <ranges>
#include <queue>
#include <execution>
#include <algorithm>
#include <future>
#include <iostream>

namespace Syn
{
    std::optional<RawModel> AssimpLoader::LoadFile(const std::filesystem::path& path)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(path.string(), AssimpUtils::ImportFlags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cerr << "[AssimpLoader] Failed to load model: " << path << "\n";
            return std::nullopt;
        }

        RawModel model{};

        PreProcessSceneHierarchy(scene, model);
        ProcessSceneHierarchy(scene, model);

        MeshUtils::LogRawModel(path, model);

        return model;
    }

    std::vector<std::string> AssimpLoader::GetSupportedExtensions() const
    {
        return { ".obj", ".fbx", ".gltf", ".glb", ".dae", ".blend" };
    }

    void AssimpLoader::PreProcessSceneHierarchy(const aiScene* scene, RawModel& outModel)
    {
        outModel.meshes.resize(scene->mNumMeshes);

        for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
        {
            aiMesh* ai_mesh = scene->mMeshes[i];
            outModel.meshes[i].name = ai_mesh->mName.C_Str();
            outModel.meshes[i].materialIndex = ai_mesh->mMaterialIndex;
            outModel.meshes[i].vertices.resize(ai_mesh->mNumVertices);
            outModel.meshes[i].indices.resize(ai_mesh->mNumFaces * 3);
        }

        outModel.materialNames.resize(scene->mNumMaterials);

        std::queue<std::pair<aiNode*, uint16_t>> queue;
        queue.push({ scene->mRootNode, UINT16_MAX });

        while (!queue.empty())
        {
            auto [currentNode, parentNodeIndex] = queue.front();
            queue.pop();

            uint16_t nodeIndex = static_cast<uint16_t>(outModel.nodeTransforms.size());

            TransformNode rawNode;
            rawNode.localTransform = AssimpUtils::ConvertAssimpToGlm(currentNode->mTransformation);

            const glm::mat4& parentTransform = parentNodeIndex != UINT16_MAX ? outModel.nodeTransforms[parentNodeIndex].globalTransform : glm::mat4(1);
            rawNode.globalTransform = parentTransform * rawNode.localTransform;
            rawNode.globalTransformIT = glm::transpose(glm::inverse(rawNode.globalTransform));
            
            outModel.nodeTransforms.push_back(rawNode);

            for (uint32_t i = 0; i < currentNode->mNumMeshes; ++i)
            {
                uint32_t meshIndex = currentNode->mMeshes[i];
                aiMesh* ai_mesh = scene->mMeshes[meshIndex];

                MeshInstanceDescriptor descriptor{};
                descriptor.meshIndex = static_cast<uint16_t>(meshIndex);
                descriptor.nodeIndex = nodeIndex;
                descriptor.parentNodeIndex = parentNodeIndex;
                descriptor.vertexCount = ai_mesh->mNumVertices;
                descriptor.indexCount = ai_mesh->mNumFaces * 3;

                outModel.meshNodeDescriptors.push_back(descriptor);
            }

            for (uint32_t i = 0; i < currentNode->mNumChildren; ++i)
                queue.push({ currentNode->mChildren[i], nodeIndex });
        }
    }

    void AssimpLoader::ProcessSceneHierarchy(const aiScene* scene, RawModel& outModel)
    {
        ProcessMaterials(scene, outModel);
        ProcessMeshVertices(scene, outModel);
        ProcessMeshIndices(scene, outModel);
    }

    void AssimpLoader::ProcessMaterials(const aiScene* scene, RawModel& outModel)
    {
        auto material_range = std::views::iota(0u, scene->mNumMaterials);
        std::for_each(std::execution::seq, material_range.begin(), material_range.end(),
            [&](uint32_t matIndex) {
                aiMaterial* matAI = scene->mMaterials[matIndex];
                outModel.materialNames[matIndex] = matAI->GetName().C_Str();
				//Todo: Process material properties (textures, colors, etc.)??
            }
        );
    }

    void AssimpLoader::ProcessMeshVertices(const aiScene* scene, RawModel& outModel)
    {
        auto mesh_range = std::views::iota(0u, scene->mNumMeshes);
        std::for_each(std::execution::seq, mesh_range.begin(), mesh_range.end(),
            [&](uint32_t meshIndex) {
                aiMesh* ai_mesh = scene->mMeshes[meshIndex];
                RawMesh& raw_mesh = outModel.meshes[meshIndex];
				raw_mesh.hasNormals = ai_mesh->HasNormals();
				raw_mesh.hasTangents = ai_mesh->HasTangentsAndBitangents();

                for (uint32_t v = 0; v < ai_mesh->mNumVertices; ++v)
                {
                    Vertex vertex{};

                    if (ai_mesh->HasPositions())
                        vertex.position = AssimpUtils::ConvertAssimpToGlm(ai_mesh->mVertices[v]);

                    if (ai_mesh->HasNormals())
                        vertex.normal = AssimpUtils::ConvertAssimpToGlm(ai_mesh->mNormals[v]);

                    if (ai_mesh->HasTangentsAndBitangents())
                        vertex.tangent = AssimpUtils::ConvertAssimpToGlm(ai_mesh->mTangents[v]);

                    if (ai_mesh->HasTextureCoords(0))
                        vertex.uv = glm::vec2(ai_mesh->mTextureCoords[0][v].x, ai_mesh->mTextureCoords[0][v].y);

                    raw_mesh.vertices[v] = vertex;
                }
            }
        );
    }

    void AssimpLoader::ProcessMeshIndices(const aiScene* scene, RawModel& outModel)
    {
        auto mesh_range = std::views::iota(0u, scene->mNumMeshes);
        std::for_each(std::execution::seq, mesh_range.begin(), mesh_range.end(),
            [&](uint32_t meshIndex) {
                aiMesh* ai_mesh = scene->mMeshes[meshIndex];
                RawMesh& raw_mesh = outModel.meshes[meshIndex];

                for (uint32_t f = 0; f < ai_mesh->mNumFaces; ++f)
                {
                    const aiFace& face = ai_mesh->mFaces[f];
                    uint32_t offset = f * 3;

                    raw_mesh.indices[offset + 0] = face.mIndices[0];
                    raw_mesh.indices[offset + 1] = face.mIndices[1];
                    raw_mesh.indices[offset + 2] = face.mIndices[2];
                }
            }
        );
    }
}