#include "AssimpAnimationLoader.h"
#include "Engine/Utils/AssimpUtils.h"
#include "Engine/ServiceLocator.h"
#include <Assimp/Importer.hpp>
#include <queue>
#include <iostream>

#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

#include "Engine/Logger/SynLog.h"

namespace Syn
{
    std::optional<RawAnimation> AssimpAnimationLoader::LoadFile(const std::filesystem::path& path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path.string(), AssimpUtils::ImportFlags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            Error("Failed to load animation:: {}", path.string());
            return std::nullopt;
        }

        if (!scene->HasAnimations())
        {
            Error("No animation tracks found in file: {}", path.string());
            return std::nullopt;
        }

        RawAnimation rawAnim{};
        aiAnimation* aiAnim = scene->mAnimations[0];

        rawAnim.name = aiAnim->mName.C_Str();
        rawAnim.duration = static_cast<float>(aiAnim->mDuration);
        rawAnim.ticksPerSecond = aiAnim->mTicksPerSecond != 0.0 ? static_cast<float>(aiAnim->mTicksPerSecond) : 25.0f;

        BuildLinearHierarchy(scene, rawAnim);

        std::unordered_map<std::string, uint32_t> nodeNameMap;
        for (uint32_t i = 0; i < rawAnim.nodes.size(); ++i) {
            nodeNameMap[rawAnim.nodes[i].name] = i;
        }

        tf::Taskflow taskflow;

        ProcessAnimationTracks(scene, rawAnim, nodeNameMap, taskflow);
        ProcessBoneWeights(scene, rawAnim, nodeNameMap, taskflow);

        ServiceLocator::GetTaskExecutor()->run(taskflow).wait();

        return rawAnim;
    }

    std::vector<std::string> AssimpAnimationLoader::GetSupportedExtensions() const
    {
        return { ".fbx", ".dae", ".gltf", ".glb", ".dae" };
    }

    void AssimpAnimationLoader::BuildLinearHierarchy(const aiScene* scene, RawAnimation& outAnim)
    {
        std::queue<std::pair<aiNode*, uint32_t>> queue;
        queue.push({ scene->mRootNode, UINT32_MAX });

        while (!queue.empty())
        {
            auto [currentNode, parentNodeIndex] = queue.front();
            queue.pop();

            uint32_t nodeIndex = static_cast<uint32_t>(outAnim.nodes.size());

            AnimationNode rawNode;
            rawNode.name = currentNode->mName.C_Str();
            rawNode.parentIndex = parentNodeIndex;

            outAnim.nodes.push_back(rawNode);

            for (uint32_t i = 0; i < currentNode->mNumChildren; ++i)
                queue.push({ currentNode->mChildren[i], nodeIndex });
        }
    }

    void AssimpAnimationLoader::ProcessAnimationTracks(const aiScene* scene, RawAnimation& outAnim, const std::unordered_map<std::string, uint32_t>& nodeNameMap, tf::Taskflow& taskflow)
    {
        tf::GuidedPartitioner partitioner(1);
        
        aiAnimation* aiAnim = scene->mAnimations[0];
        outAnim.tracks.resize(aiAnim->mNumChannels);

        taskflow.for_each_index(0u, aiAnim->mNumChannels, 1u,
            [&, aiAnim](uint32_t channelIndex) {
                aiNodeAnim* channel = aiAnim->mChannels[channelIndex];
                BoneTrack& track = outAnim.tracks[channelIndex];

                track.nodeName = channel->mNodeName.C_Str();

                track.positions.reserve(channel->mNumPositionKeys);
                for (uint32_t p = 0; p < channel->mNumPositionKeys; ++p) {
                    track.positions.push_back({
                        static_cast<float>(channel->mPositionKeys[p].mTime),
                        AssimpUtils::ConvertAssimpToGlm(channel->mPositionKeys[p].mValue)
                        });
                }

                track.rotations.reserve(channel->mNumRotationKeys);
                for (uint32_t r = 0; r < channel->mNumRotationKeys; ++r) {
                    auto aiQuat = channel->mRotationKeys[r].mValue;
                    track.rotations.push_back({
                        static_cast<float>(channel->mRotationKeys[r].mTime),
                        glm::quat(aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z)
                        });
                }

                track.scales.reserve(channel->mNumScalingKeys);
                for (uint32_t s = 0; s < channel->mNumScalingKeys; ++s) {
                    track.scales.push_back({
                        static_cast<float>(channel->mScalingKeys[s].mTime),
                        AssimpUtils::ConvertAssimpToGlm(channel->mScalingKeys[s].mValue)
                        });
                }

                auto it = nodeNameMap.find(track.nodeName);
                if (it != nodeNameMap.end()) {
                    outAnim.nodes[it->second].trackIndex = channelIndex;
                }
            }, 
            partitioner
        );
    }

    void AssimpAnimationLoader::ProcessBoneWeights(const aiScene* scene, RawAnimation& outAnim, const std::unordered_map<std::string, uint32_t>& nodeNameMap, tf::Taskflow& taskflow)
    {
        tf::GuidedPartitioner partitioner(1);

        outAnim.meshSkins.resize(scene->mNumMeshes);

        taskflow.for_each_index(0u, scene->mNumMeshes, 1u,
            [&, scene](uint32_t meshIndex) {
                aiMesh* ai_mesh = scene->mMeshes[meshIndex];
                RawMeshSkin& rawSkin = outAnim.meshSkins[meshIndex];

                rawSkin.vertices.resize(ai_mesh->mNumVertices);

                for (uint32_t b = 0; b < ai_mesh->mNumBones; ++b) {
                    aiBone* bone = ai_mesh->mBones[b];
                    std::string boneName = bone->mName.C_Str();

                    uint32_t nodeIndex = UINT32_MAX;
                    auto it = nodeNameMap.find(boneName);

                    if (it != nodeNameMap.end()) {
                        nodeIndex = it->second;
                        outAnim.nodes[nodeIndex].offsetMatrix = AssimpUtils::ConvertAssimpToGlm(bone->mOffsetMatrix);
                    }
                    else {
                        continue;
                    }

                    for (uint32_t w = 0; w < bone->mNumWeights; ++w) {
                        uint32_t localVertexId = bone->mWeights[w].mVertexId;
                        float weight = bone->mWeights[w].mWeight;

                        for (int i = 0; i < 4; ++i) {
                            if (rawSkin.vertices[localVertexId].boneIndices[i] == UINT32_MAX) {
                                rawSkin.vertices[localVertexId].boneIndices[i] = nodeIndex;
                                rawSkin.vertices[localVertexId].boneWeights[i] = weight;
                                break;
                            }
                        }
                    }
                }
            },
            partitioner
        );
    }
}