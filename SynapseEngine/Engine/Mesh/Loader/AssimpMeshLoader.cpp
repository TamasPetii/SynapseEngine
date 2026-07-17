#include "AssimpMeshLoader.h"
#include "Engine/Utils/AssimpUtils.h"
#include "Engine/Mesh/Utils/MeshUtils.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <ranges>
#include <queue>
#include <execution>
#include <algorithm>
#include <future>
#include <iostream>

#include "Engine/ServiceLocator.h"

#include <chrono>
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

            if (currentNode->mNumMeshes == 0)
            {
                MeshInstanceDescriptor descriptor{};
                descriptor.name = currentNode->mName.C_Str();
                descriptor.meshIndex = UINT16_MAX;
                descriptor.nodeIndex = nodeIndex;
                descriptor.parentNodeIndex = parentNodeIndex;
                descriptor.vertexCount = 0;
                descriptor.indexCount = 0;
                outModel.meshNodeDescriptors.push_back(descriptor);
            }
            else
            {
                for (uint32_t i = 0; i < currentNode->mNumMeshes; ++i)
                {
                    uint32_t meshIndex = currentNode->mMeshes[i];
                    aiMesh* ai_mesh = scene->mMeshes[meshIndex];
                    MeshInstanceDescriptor descriptor{};

                    descriptor.name = currentNode->mName.C_Str();

                    if (currentNode->mNumMeshes > 1) {
                        descriptor.name += "_" + std::to_string(i);
                    }

                    descriptor.meshIndex = static_cast<uint16_t>(meshIndex);
                    descriptor.nodeIndex = nodeIndex;
                    descriptor.parentNodeIndex = parentNodeIndex;
                    descriptor.vertexCount = ai_mesh->mNumVertices;
                    descriptor.indexCount = ai_mesh->mNumFaces * 3;
                    outModel.meshNodeDescriptors.push_back(descriptor);
                }

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

        ServiceLocator::Get<tf::Executor>()->run(taskflow).wait();
    }

    void AssimpMeshLoader::ProcessMaterials(const aiScene* scene, RawModel& outModel, tf::Taskflow& taskflow)
    {
        taskflow.for_each_index(0u, scene->mNumMaterials, 1u, 
            [&, scene](uint32_t matIndex) {
                aiMaterial* matAI = scene->mMaterials[matIndex];
                MaterialInfo& matInfo = outModel.materials[matIndex];

                matInfo.name = matAI->GetName().C_Str();

                auto extractTexture = [&](aiTextureType type, TexturePayload& outPayload) -> bool {
                    if (matAI->GetTextureCount(type) > 0) {

                        aiString path;
                        matAI->GetTexture(type, 0, &path);
                        outPayload.path = path.C_Str();

                        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(path.C_Str());

                        if (embeddedTexture) {
                            if (embeddedTexture->mHeight == 0) {
                                size_t dataSize = embeddedTexture->mWidth;
                                outPayload.embeddedData.assign(
                                    (uint8_t*)embeddedTexture->pcData,
                                    (uint8_t*)embeddedTexture->pcData + dataSize
                                );
                                outPayload.isUncompressed = false;
                                outPayload.formatHint = embeddedTexture->achFormatHint;
                            }
                            else {
                                size_t dataSize = embeddedTexture->mWidth * embeddedTexture->mHeight * 4;
                                outPayload.embeddedData.assign(
                                    (uint8_t*)embeddedTexture->pcData,
                                    (uint8_t*)embeddedTexture->pcData + dataSize
                                );
                                outPayload.width = embeddedTexture->mWidth;
                                outPayload.height = embeddedTexture->mHeight;
                                outPayload.isUncompressed = true;
                            }
                        }
                        return true;
                    }
                    return false;
                    };


                if (!extractTexture(aiTextureType_DIFFUSE, matInfo.albedo)) {
                    extractTexture(aiTextureType_BASE_COLOR, matInfo.albedo);
                }

                if (!extractTexture(aiTextureType_NORMALS, matInfo.normal)) {
                    if (!extractTexture(aiTextureType_HEIGHT, matInfo.normal)) {
						if(!extractTexture(aiTextureType_DISPLACEMENT, matInfo.normal)) {
	                        
						}
                    }
                }

                if (!extractTexture(aiTextureType_METALNESS, matInfo.metallicRoughness)) {
                    extractTexture(aiTextureType_UNKNOWN, matInfo.metallicRoughness);
                }

                if (!extractTexture(aiTextureType_EMISSIVE, matInfo.emissive)) {
                }

                if (!extractTexture(aiTextureType_LIGHTMAP, matInfo.ambientOcclusion)) {
                    extractTexture(aiTextureType_AMBIENT, matInfo.ambientOcclusion);
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

                int twoSided = 0;
                if (AI_SUCCESS == matAI->Get(AI_MATKEY_TWOSIDED, twoSided)) {
                    matInfo.doubleSided = (twoSided != 0);
                }

                ai_real opacity = 1.0f;
                if (AI_SUCCESS == matAI->Get(AI_MATKEY_OPACITY, opacity)) {
                    if (false && opacity < 0.98f) {
                        matInfo.isTransparent = true;
                    }
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