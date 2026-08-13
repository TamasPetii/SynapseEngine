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

#include "TinyGltfAnimationLoader.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/ServiceLocator.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <queue>
#include <mutex>

#define TINYGLTF_NO_STB_IMAGE_IMPLEMENTATION 
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

namespace Syn
{
    bool DummyAnimImageLoader(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*) {
        return true;
    }

    std::vector<std::string> TinyGltfAnimationLoader::GetSupportedExtensions() const {
        return { ".gltf", ".glb" };
    }

    std::optional<RawAnimation> TinyGltfAnimationLoader::LoadFile(const std::filesystem::path& path) {
        tinygltf::Model gltfModel;
        tinygltf::TinyGLTF loader;
        std::string err, warn;

        loader.SetImageLoader(DummyAnimImageLoader, nullptr);

        bool ret = path.extension() == ".glb" ?
            loader.LoadBinaryFromFile(&gltfModel, &err, &warn, path.string()) :
            loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path.string());

        if (!warn.empty()) Warning("TinyGltf Anim: {}", warn);
        if (!err.empty()) Error("TinyGltf Anim: {}", err);
        if (!ret || gltfModel.animations.empty()) {
            Error("Failed to load or no animations found in: {}", path.string());
            return std::nullopt;
        }

        RawAnimation rawAnim{};
        const auto& anim = gltfModel.animations[0];

        rawAnim.name = anim.name.empty() ? "Animation_0" : anim.name;
        rawAnim.ticksPerSecond = 1.0f;

        std::vector<uint32_t> gltfToFlatNode(gltfModel.nodes.size(), UINT32_MAX);

        BuildLinearHierarchy(gltfModel, rawAnim, gltfToFlatNode);

        _totalPrimitives = 0;
        _meshPrimitiveToRawMeshIndex.resize(gltfModel.meshes.size());
        for (size_t i = 0; i < gltfModel.meshes.size(); ++i) {
            _meshPrimitiveToRawMeshIndex[i].resize(gltfModel.meshes[i].primitives.size());
            for (size_t j = 0; j < gltfModel.meshes[i].primitives.size(); ++j) {
                _meshPrimitiveToRawMeshIndex[i][j] = _totalPrimitives++;
            }
        }

        tf::Taskflow taskflow;

        ProcessAnimationTracks(gltfModel, rawAnim, gltfToFlatNode, taskflow);
        ProcessBoneWeights(gltfModel, rawAnim, gltfToFlatNode, taskflow);

        ServiceLocator::Get<tf::Executor>()->run(taskflow).wait();

        return rawAnim;
    }

    void TinyGltfAnimationLoader::BuildLinearHierarchy(const tinygltf::Model& gltfModel, RawAnimation& outAnim, std::vector<uint32_t>& outGltfToFlatNode) {
        if (gltfModel.scenes.empty()) return;

        const auto& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
        outAnim.nodes.reserve(gltfModel.nodes.size());

        std::queue<std::pair<int, uint32_t>> queue;
        for (int rootNodeIdx : scene.nodes) queue.push({ rootNodeIdx, UINT32_MAX });

        while (!queue.empty()) {
            auto [gltfNodeIdx, parentNodeIndex] = queue.front();
            queue.pop();

            const auto& node = gltfModel.nodes[gltfNodeIdx];
            uint32_t currentNodeIndex = static_cast<uint32_t>(outAnim.nodes.size());
            outGltfToFlatNode[gltfNodeIdx] = currentNodeIndex;

            AnimationNode rawNode;
            rawNode.name = node.name.empty() ? ("Node_" + std::to_string(gltfNodeIdx)) : node.name;
            rawNode.parentIndex = parentNodeIndex;

            // Reconstruct default local transform
            if (node.matrix.size() == 16) {
                float mat[16];
                for (int i = 0; i < 16; ++i) mat[i] = static_cast<float>(node.matrix[i]);
                rawNode.defaultLocalTransform = glm::make_mat4(mat);
            }
            else {
                glm::vec3 t(0.0f), s(1.0f);
                glm::quat r(1.0f, 0.0f, 0.0f, 0.0f);

                if (node.translation.size() == 3) t = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
                if (node.rotation.size() == 4) r = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
                if (node.scale.size() == 3) s = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);

                rawNode.defaultLocalTransform = glm::translate(glm::mat4(1.0f), t) * glm::mat4_cast(r) * glm::scale(glm::mat4(1.0f), s);
            }

            outAnim.nodes.push_back(rawNode);

            for (int childIdx : node.children) {
                queue.push({ childIdx, currentNodeIndex });
            }
        }
    }

    void TinyGltfAnimationLoader::ProcessAnimationTracks(const tinygltf::Model& gltfModel, RawAnimation& outAnim, const std::vector<uint32_t>& gltfToFlatNode, tf::Taskflow& taskflow) {
        const auto& anim = gltfModel.animations[0];
        outAnim.tracks.resize(anim.channels.size());

        std::mutex durationMutex;
        tf::GuidedPartitioner partitioner(1);

        taskflow.for_each_index(0u, static_cast<uint32_t>(anim.channels.size()), 1u,
            [&, anim](uint32_t channelIndex) {
                const auto& channel = anim.channels[channelIndex];
                const auto& sampler = anim.samplers[channel.sampler];
                BoneTrack& track = outAnim.tracks[channelIndex];

                uint32_t targetFlatNodeIndex = gltfToFlatNode[channel.target_node];
                track.nodeName = outAnim.nodes[targetFlatNodeIndex].name;
                outAnim.nodes[targetFlatNodeIndex].trackIndex = channelIndex;

                // Extract keyframe timestamps (Input)
                const auto& inputAccessor = gltfModel.accessors[sampler.input];
                const auto& inputView = gltfModel.bufferViews[inputAccessor.bufferView];
                const float* times = reinterpret_cast<const float*>(gltfModel.buffers[inputView.buffer].data.data() + inputView.byteOffset + inputAccessor.byteOffset);

                // Extract keyframe values (Output)
                const auto& outputAccessor = gltfModel.accessors[sampler.output];
                const auto& outputView = gltfModel.bufferViews[outputAccessor.bufferView];
                const uint8_t* outData = gltfModel.buffers[outputView.buffer].data.data() + outputView.byteOffset + outputAccessor.byteOffset;
                size_t outStride = outputAccessor.ByteStride(outputView);

                float maxTime = times[inputAccessor.count - 1];
                {
                    std::lock_guard<std::mutex> lock(durationMutex);
                    if (maxTime > outAnim.duration) outAnim.duration = maxTime;
                }

                if (channel.target_path == "translation") {
                    track.positions.reserve(inputAccessor.count);
                    for (size_t i = 0; i < inputAccessor.count; ++i) {
                        const float* val = reinterpret_cast<const float*>(outData + i * outStride);
                        track.positions.push_back({ times[i], glm::vec3(val[0], val[1], val[2]) });
                    }
                }
                else if (channel.target_path == "rotation") {
                    track.rotations.reserve(inputAccessor.count);
                    for (size_t i = 0; i < inputAccessor.count; ++i) {
                        const float* val = reinterpret_cast<const float*>(outData + i * outStride);
                        track.rotations.push_back({ times[i], glm::quat(val[3], val[0], val[1], val[2]) });
                    }
                }
                else if (channel.target_path == "scale") {
                    track.scales.reserve(inputAccessor.count);
                    for (size_t i = 0; i < inputAccessor.count; ++i) {
                        const float* val = reinterpret_cast<const float*>(outData + i * outStride);
                        track.scales.push_back({ times[i], glm::vec3(val[0], val[1], val[2]) });
                    }
                }
            }, partitioner);
    }

    void TinyGltfAnimationLoader::ProcessBoneWeights(const tinygltf::Model& gltfModel, RawAnimation& outAnim, const std::vector<uint32_t>& gltfToFlatNode, tf::Taskflow& taskflow) {
        if (_totalPrimitives == 0) return;
        outAnim.meshSkins.resize(_totalPrimitives);

        for (size_t n = 0; n < gltfModel.nodes.size(); ++n) {
            const auto& node = gltfModel.nodes[n];
            if (node.mesh < 0 || node.skin < 0) continue;

            const auto& skin = gltfModel.skins[node.skin];
            const auto& gltfMesh = gltfModel.meshes[node.mesh];

            if (skin.inverseBindMatrices >= 0) {
                const auto& acc = gltfModel.accessors[skin.inverseBindMatrices];
                const auto& view = gltfModel.bufferViews[acc.bufferView];
                const float* ibmData = reinterpret_cast<const float*>(gltfModel.buffers[view.buffer].data.data() + view.byteOffset + acc.byteOffset);

                for (size_t j = 0; j < skin.joints.size(); ++j) {
                    uint32_t flatNodeIndex = gltfToFlatNode[skin.joints[j]];
                    if (flatNodeIndex != UINT32_MAX) {
                        float mat[16];
                        for (int m = 0; m < 16; ++m) mat[m] = ibmData[j * 16 + m];
                        outAnim.nodes[flatNodeIndex].offsetMatrix = glm::make_mat4(mat);
                    }
                }
            }

            tf::GuidedPartitioner partitioner(1);
            taskflow.for_each_index(0u, static_cast<uint32_t>(gltfMesh.primitives.size()), 1u,
                [&, node, skin, gltfMesh](uint32_t pIdx) {
                    const auto& prim = gltfMesh.primitives[pIdx];
                    uint32_t rawMeshIndex = _meshPrimitiveToRawMeshIndex[node.mesh][pIdx];
                    RawMeshSkin& rawSkin = outAnim.meshSkins[rawMeshIndex];

                    auto getAccessor = [&](const std::string& attr, size_t& count, size_t& stride, const uint8_t*& ptr, int& compType) -> bool {
                        if (prim.attributes.find(attr) == prim.attributes.end()) return false;
                        const auto& acc = gltfModel.accessors[prim.attributes.at(attr)];
                        const auto& view = gltfModel.bufferViews[acc.bufferView];
                        count = acc.count;
                        stride = acc.ByteStride(view);
                        compType = acc.componentType;
                        ptr = gltfModel.buffers[view.buffer].data.data() + view.byteOffset + acc.byteOffset;
                        return true;
                        };

                    size_t jCount, jStride; const uint8_t* jPtr; int jType;
                    size_t wCount, wStride; const uint8_t* wPtr; int wType;

                    if (getAccessor("JOINTS_0", jCount, jStride, jPtr, jType) && getAccessor("WEIGHTS_0", wCount, wStride, wPtr, wType)) {
                        rawSkin.vertices.resize(jCount);

                        for (size_t i = 0; i < jCount; ++i) {
                            VertexSkinData& vSkin = rawSkin.vertices[i];
                            const uint8_t* jBase = jPtr + i * jStride;
                            const uint8_t* wBase = wPtr + i * wStride;

                            // Map joint index to flat node index
                            for (int k = 0; k < 4; ++k) {
                                uint32_t jointIndex = (jType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) ? jBase[k] : reinterpret_cast<const uint16_t*>(jBase)[k];
                                vSkin.boneIndices[k] = gltfToFlatNode[skin.joints[jointIndex]];
                            }

                            // Normalize weights based on component type
                            for (int k = 0; k < 4; ++k) {
                                if (wType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                                    vSkin.boneWeights[k] = reinterpret_cast<const float*>(wBase)[k];
                                }
                                else if (wType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                                    vSkin.boneWeights[k] = reinterpret_cast<const uint16_t*>(wBase)[k] / 65535.0f;
                                }
                                else if (wType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                                    vSkin.boneWeights[k] = wBase[k] / 255.0f;
                                }
                            }
                        }
                    }
                }, partitioner);
        }
    }
}