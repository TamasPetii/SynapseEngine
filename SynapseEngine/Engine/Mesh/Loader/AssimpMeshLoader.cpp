#include "AssimpMeshLoader.h"
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

#include "Engine/ServiceLocator.h"
#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

#include "Engine/Logger/SynLog.h"

namespace Syn
{
    std::optional<RawModel> AssimpMeshLoader::LoadFile(const std::filesystem::path& path)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(path.string(), AssimpUtils::ImportFlags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            Error("Failed to load model: {}", path.string());
            return std::nullopt;
        }

        RawModel model{};

        PreProcessSceneHierarchy(scene, model);
        ProcessSceneHierarchy(scene, model);

        return model;
    }

    std::vector<std::string> AssimpMeshLoader::GetSupportedExtensions() const
    {
        return { ".obj", ".fbx", ".gltf", ".glb", ".dae", ".blend" };
    }

    void AssimpMeshLoader::PreProcessSceneHierarchy(const aiScene* scene, RawModel& outModel)
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

        outModel.materials.resize(scene->mNumMaterials);

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

    void AssimpMeshLoader::ProcessSceneHierarchy(const aiScene* scene, RawModel& outModel)
    {
        tf::Taskflow taskflow;

        ProcessMaterials(scene, outModel, taskflow);
        ProcessMeshVertices(scene, outModel, taskflow);
        ProcessMeshIndices(scene, outModel, taskflow);

        ServiceLocator::GetTaskExecutor()->run(taskflow).wait();
    }

    void AssimpMeshLoader::ProcessMaterials(const aiScene* scene, RawModel& outModel, tf::Taskflow& taskflow)
    {
        taskflow.for_each_index(0u, scene->mNumMaterials, 1u, 
            [&, scene](uint32_t matIndex) {
                aiMaterial* matAI = scene->mMaterials[matIndex];
                MaterialInfo& matInfo = outModel.materials[matIndex];

                matInfo.name = matAI->GetName().C_Str();

                aiString path;

                if (matAI->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                    matAI->GetTexture(aiTextureType_DIFFUSE, 0, &path);
                    matInfo.albedoPath = path.C_Str();
                }
                else if (matAI->GetTextureCount(aiTextureType_BASE_COLOR) > 0) {
                    matAI->GetTexture(aiTextureType_BASE_COLOR, 0, &path);
                    matInfo.albedoPath = path.C_Str();
                }

                if (matAI->GetTextureCount(aiTextureType_NORMALS) > 0) {
                    matAI->GetTexture(aiTextureType_NORMALS, 0, &path);
                    matInfo.normalPath = path.C_Str();
                }

                if (matAI->GetTextureCount(aiTextureType_METALNESS) > 0) {
                    matAI->GetTexture(aiTextureType_METALNESS, 0, &path);
                    matInfo.metallicRoughnessPath = path.C_Str();
                }
                else if (matAI->GetTextureCount(aiTextureType_UNKNOWN) > 0) {
                    matAI->GetTexture(aiTextureType_UNKNOWN, 0, &path);
                    matInfo.metallicRoughnessPath = path.C_Str();
                }

                if (matAI->GetTextureCount(aiTextureType_EMISSIVE) > 0) {
                    matAI->GetTexture(aiTextureType_EMISSIVE, 0, &path);
                    matInfo.emissivePath = path.C_Str();
                }

                if (matAI->GetTextureCount(aiTextureType_LIGHTMAP) > 0) {
                    matAI->GetTexture(aiTextureType_LIGHTMAP, 0, &path);
                    matInfo.ambientOcclusionPath = path.C_Str();
                }
                else if (matAI->GetTextureCount(aiTextureType_AMBIENT) > 0) {
                    matAI->GetTexture(aiTextureType_AMBIENT, 0, &path);
                    matInfo.ambientOcclusionPath = path.C_Str();
                }

                aiColor4D color;

                if (AI_SUCCESS == matAI->Get(AI_MATKEY_BASE_COLOR, color)) {
                    matInfo.color = glm::vec4(color.r, color.g, color.b, color.a);
                }
                else if (AI_SUCCESS == matAI->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
                    matInfo.color = glm::vec4(color.r, color.g, color.b, color.a);
                }

                aiColor3D emissive;
                if (AI_SUCCESS == matAI->Get(AI_MATKEY_COLOR_EMISSIVE, emissive)) {
                    matInfo.emissiveFactor = glm::vec3(emissive.r, emissive.g, emissive.b);
                }

                ai_real floatValue;
                if (AI_SUCCESS == matAI->Get(AI_MATKEY_METALLIC_FACTOR, floatValue)) {
                    matInfo.metallicFactor = static_cast<float>(floatValue);
                }

                if (AI_SUCCESS == matAI->Get(AI_MATKEY_ROUGHNESS_FACTOR, floatValue)) {
                    matInfo.roughnessFactor = static_cast<float>(floatValue);
                }
            }
        );
    }

    void AssimpMeshLoader::ProcessMeshVertices(const aiScene* scene, RawModel& outModel, tf::Taskflow& taskflow)
    {
        tf::GuidedPartitioner partitioner(1);

        taskflow.for_each_index(0u, scene->mNumMeshes, 1u, 
            [&, scene](uint32_t meshIndex) {
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
            }, 
            partitioner
        );
    }

    void AssimpMeshLoader::ProcessMeshIndices(const aiScene* scene, RawModel& outModel, tf::Taskflow& taskflow)
    {
        tf::GuidedPartitioner partitioner(1);

        taskflow.for_each_index(0u, scene->mNumMeshes, 1u, 
            [&, scene](uint32_t meshIndex) {
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
            },
            partitioner
        );
    }
}