#include "TinyGltfLoader.h"
#include "Engine/Logger/SynLog.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Image/SamplerNames.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <queue>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_IMPLEMENTATION 
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

namespace Syn
{
    bool DummyImageLoader(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*) {
        return true;
    }

    std::vector<std::string> TinyGltfLoader::GetSupportedExtensions() const {
        return { ".gltf", ".glb" };
    }

    std::optional<RawModel> TinyGltfLoader::LoadFile(const std::filesystem::path& path) {
        tinygltf::Model gltfModel;
        tinygltf::TinyGLTF loader;
        std::string err, warn;

        // Force tinygltf to NOT decode images on the main thread
        loader.SetImageLoader(DummyImageLoader, nullptr);

        bool ret = false;
        if (path.extension() == ".glb") {
            ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, path.string());
        }
        else {
            ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path.string());
        }

        if (!warn.empty()) Warning("TinyGltf: {}", warn);
        if (!err.empty()) Error("TinyGltf: {}", err);
        if (!ret) {
            Error("Failed to load glTF model: {}", path.string());
            return std::nullopt;
        }

        RawModel model{};
        tf::Taskflow taskflow;

        ProcessMaterials(gltfModel, path.parent_path(), model, taskflow);
        ProcessMeshes(gltfModel, model, taskflow);

        ServiceLocator::Get<tf::Executor>()->run(taskflow).wait();

        ProcessNodes(gltfModel, model);

        return model;
    }

    void TinyGltfLoader::ProcessMaterials(const tinygltf::Model& gltfModel, const std::filesystem::path& basePath, RawModel& outModel, tf::Taskflow& taskflow) {
        outModel.materials.resize(gltfModel.materials.size());

        if (gltfModel.materials.empty()) return;

        taskflow.for_each_index(0u, static_cast<uint32_t>(gltfModel.materials.size()), 1u,
            [&, gltfModel, basePath](uint32_t matIndex) {
                const auto& matGltf = gltfModel.materials[matIndex];
                MaterialInfo& matInfo = outModel.materials[matIndex];

                matInfo.name = matGltf.name;
                matInfo.doubleSided = matGltf.doubleSided;

                if (matGltf.alphaMode == "BLEND") {
                    matInfo.isTransparent = true;
                }
                else if (matGltf.alphaMode == "MASK") {
                    matInfo.isAlphaTested = true;
                }

                auto extractTexture = [&](int texIndex, TexturePayload& outPayload) {
                    if (texIndex < 0 || texIndex >= gltfModel.textures.size()) return;

                    const auto& texture = gltfModel.textures[texIndex];
                    const auto& image = gltfModel.images[texture.source];

                    // Resolve Sampler parameters
                    if (texture.sampler >= 0) {
                        const auto& sampler = gltfModel.samplers[texture.sampler];
                        auto mapModeToStr = [](int wrap) {
                            switch (wrap) {
                            case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: return SamplerWrapModeNames::ClampEdge;
                            case TINYGLTF_TEXTURE_WRAP_REPEAT: return SamplerWrapModeNames::Repeat;
                            default: return SamplerWrapModeNames::Repeat;
                            }
                            };
                        outPayload.wrapModeU = mapModeToStr(sampler.wrapS);
                        outPayload.wrapModeV = mapModeToStr(sampler.wrapT);
                    }

                    // Handle embedded binary texture (.glb) vs URI external file (.gltf)
                    if (image.bufferView >= 0) {
                        const auto& bufferView = gltfModel.bufferViews[image.bufferView];
                        const auto& buffer = gltfModel.buffers[bufferView.buffer];

                        outPayload.embeddedData.assign(
                            buffer.data.begin() + bufferView.byteOffset,
                            buffer.data.begin() + bufferView.byteOffset + bufferView.byteLength
                        );
                        outPayload.isUncompressed = false;
                        outPayload.formatHint = image.mimeType == "image/jpeg" ? "jpg" : "png";
                        outPayload.path = matInfo.name + "_tex_" + std::to_string(texIndex);
                    }
                    else if (!image.uri.empty()) {
                        outPayload.path = (basePath / image.uri).string();
                    }
                    };

                const auto& pbr = matGltf.pbrMetallicRoughness;

                matInfo.color = glm::vec4(pbr.baseColorFactor[0], pbr.baseColorFactor[1], pbr.baseColorFactor[2], pbr.baseColorFactor[3]);
                matInfo.metallicFactor = static_cast<float>(pbr.metallicFactor);
                matInfo.roughnessFactor = static_cast<float>(pbr.roughnessFactor);
                matInfo.emissiveFactor = glm::vec3(matGltf.emissiveFactor[0], matGltf.emissiveFactor[1], matGltf.emissiveFactor[2]);

                extractTexture(pbr.baseColorTexture.index, matInfo.albedo);
                extractTexture(pbr.metallicRoughnessTexture.index, matInfo.metallicRoughness);
                extractTexture(matGltf.normalTexture.index, matInfo.normal);
                extractTexture(matGltf.occlusionTexture.index, matInfo.ambientOcclusion);
                extractTexture(matGltf.emissiveTexture.index, matInfo.emissive);

                if (matGltf.alphaMode == "MASK") {
                    extractTexture(pbr.baseColorTexture.index, matInfo.opacity);
                }
            });
    }

    void TinyGltfLoader::ProcessMeshes(const tinygltf::Model& gltfModel, RawModel& outModel, tf::Taskflow& taskflow) {
        uint32_t totalPrimitives = 0;
        _meshPrimitiveToRawMeshIndex.resize(gltfModel.meshes.size());

        // Pre-allocate flat mesh array bounds
        for (size_t i = 0; i < gltfModel.meshes.size(); ++i) {
            _meshPrimitiveToRawMeshIndex[i].resize(gltfModel.meshes[i].primitives.size());
            for (size_t j = 0; j < gltfModel.meshes[i].primitives.size(); ++j) {
                _meshPrimitiveToRawMeshIndex[i][j] = totalPrimitives++;
            }
        }

        if (totalPrimitives == 0) return;
        outModel.meshes.resize(totalPrimitives);

        tf::GuidedPartitioner partitioner(1);
        taskflow.for_each_index(0u, totalPrimitives, 1u,
            [&, gltfModel](uint32_t rawMeshIndex) {

                // Find matching glTF Mesh and Primitive index
                size_t mIdx = 0, pIdx = 0;
                bool found = false;
                for (size_t i = 0; i < _meshPrimitiveToRawMeshIndex.size() && !found; ++i) {
                    for (size_t j = 0; j < _meshPrimitiveToRawMeshIndex[i].size(); ++j) {
                        if (_meshPrimitiveToRawMeshIndex[i][j] == rawMeshIndex) {
                            mIdx = i; pIdx = j; found = true;
                        }
                    }
                }

                const auto& gltfMesh = gltfModel.meshes[mIdx];
                const auto& prim = gltfMesh.primitives[pIdx];
                RawMesh& rawMesh = outModel.meshes[rawMeshIndex];

                rawMesh.name = gltfMesh.name.empty() ? "Mesh_" + std::to_string(mIdx) : gltfMesh.name;
                if (gltfMesh.primitives.size() > 1) rawMesh.name += "_" + std::to_string(pIdx);

                rawMesh.materialIndex = prim.material >= 0 ? prim.material : 0;

                // Extract Indices
                if (prim.indices >= 0) {
                    const auto& accessor = gltfModel.accessors[prim.indices];
                    const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                    const auto& buffer = gltfModel.buffers[bufferView.buffer];

                    rawMesh.indices.resize(accessor.count);

                    // Direct pointer to binary data chunk
                    const uint8_t* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        const uint16_t* buf = reinterpret_cast<const uint16_t*>(dataPtr);
                        for (size_t i = 0; i < accessor.count; ++i) rawMesh.indices[i] = buf[i];
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                        const uint32_t* buf = reinterpret_cast<const uint32_t*>(dataPtr);
                        for (size_t i = 0; i < accessor.count; ++i) rawMesh.indices[i] = buf[i];
                    }
                    else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                        const uint8_t* buf = reinterpret_cast<const uint8_t*>(dataPtr);
                        for (size_t i = 0; i < accessor.count; ++i) rawMesh.indices[i] = buf[i];
                    }
                }

                // Helper for Zero-Copy vertex attribute extraction
                auto getAccessorData = [&](const std::string& attrName, size_t& count, size_t& stride, const uint8_t*& ptr) -> bool {
                    if (prim.attributes.find(attrName) == prim.attributes.end()) return false;
                    const auto& accessor = gltfModel.accessors[prim.attributes.at(attrName)];
                    const auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                    const auto& buffer = gltfModel.buffers[bufferView.buffer];

                    count = accessor.count;
                    stride = accessor.ByteStride(bufferView);
                    ptr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;
                    return true;
                    };

                size_t vCount, vStride; const uint8_t* vPtr;
                if (getAccessorData("POSITION", vCount, vStride, vPtr)) {
                    rawMesh.vertices.resize(vCount);
                    for (size_t i = 0; i < vCount; ++i) {
                        const float* pos = reinterpret_cast<const float*>(vPtr + i * vStride);
                        rawMesh.vertices[i].position = glm::vec3(pos[0], pos[1], pos[2]);
                    }
                }

                size_t nCount, nStride; const uint8_t* nPtr;
                rawMesh.hasNormals = getAccessorData("NORMAL", nCount, nStride, nPtr);
                if (rawMesh.hasNormals) {
                    for (size_t i = 0; i < nCount; ++i) {
                        const float* norm = reinterpret_cast<const float*>(nPtr + i * nStride);
                        rawMesh.vertices[i].normal = glm::vec3(norm[0], norm[1], norm[2]);
                    }
                }

                size_t tCount, tStride; const uint8_t* tPtr;
                rawMesh.hasTangents = getAccessorData("TANGENT", tCount, tStride, tPtr);
                if (rawMesh.hasTangents) {
                    for (size_t i = 0; i < tCount; ++i) {
                        const float* tan = reinterpret_cast<const float*>(tPtr + i * tStride);
                        rawMesh.vertices[i].tangent = glm::vec3(tan[0], tan[1], tan[2]);
                    }
                }

                size_t uvCount, uvStride; const uint8_t* uvPtr;
                if (getAccessorData("TEXCOORD_0", uvCount, uvStride, uvPtr)) {
                    for (size_t i = 0; i < uvCount; ++i) {
                        const float* uv = reinterpret_cast<const float*>(uvPtr + i * uvStride);
                        rawMesh.vertices[i].uv = glm::vec2(uv[0], uv[1]);
                    }
                }
            }, partitioner);
    }

    void TinyGltfLoader::ProcessNodes(const tinygltf::Model& gltfModel, RawModel& outModel) {
        if (gltfModel.scenes.empty()) return;

        const auto& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];

        // Process Scene Graph (BFS)
        std::queue<std::pair<int, uint16_t>> queue;
        for (int rootNodeIdx : scene.nodes) {
            queue.push({ rootNodeIdx, UINT16_MAX });
        }

        while (!queue.empty()) {
            auto [gltfNodeIdx, parentNodeIndex] = queue.front();
            queue.pop();

            const auto& node = gltfModel.nodes[gltfNodeIdx];
            uint16_t currentNodeIndex = static_cast<uint16_t>(outModel.nodeTransforms.size());

            TransformNode rawNode;

            // Reconstruct local transform from Matrix or TRS components
            if (node.matrix.size() == 16) {
                float mat[16];
                for (int i = 0; i < 16; ++i) mat[i] = static_cast<float>(node.matrix[i]);
                rawNode.localTransform = glm::make_mat4(mat);
            }
            else {
                glm::vec3 t(0.0f), s(1.0f);
                glm::quat r(1.0f, 0.0f, 0.0f, 0.0f);

                if (node.translation.size() == 3) t = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
                if (node.rotation.size() == 4) r = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
                if (node.scale.size() == 3) s = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);

                glm::mat4 tMat = glm::translate(glm::mat4(1.0f), t);
                glm::mat4 rMat = glm::mat4_cast(r);
                glm::mat4 sMat = glm::scale(glm::mat4(1.0f), s);
                rawNode.localTransform = tMat * rMat * sMat;
            }

            const glm::mat4& parentTransform = parentNodeIndex != UINT16_MAX ? outModel.nodeTransforms[parentNodeIndex].globalTransform : glm::mat4(1.0f);

            rawNode.globalTransform = parentTransform * rawNode.localTransform;
            rawNode.globalTransformIT = glm::transpose(glm::inverse(rawNode.globalTransform));
            outModel.nodeTransforms.push_back(rawNode);

            // Bind Meshes (Primitives) to this Transform Node
            if (node.mesh >= 0) {
                const auto& gltfMesh = gltfModel.meshes[node.mesh];
                for (size_t p = 0; p < gltfMesh.primitives.size(); ++p) {
                    uint32_t flatMeshIndex = _meshPrimitiveToRawMeshIndex[node.mesh][p];

                    MeshInstanceDescriptor descriptor{};
                    descriptor.name = node.name.empty() ? ("Node_" + std::to_string(gltfNodeIdx)) : node.name;
                    if (gltfMesh.primitives.size() > 1) descriptor.name += "_" + std::to_string(p);

                    descriptor.meshIndex = static_cast<uint16_t>(flatMeshIndex);
                    descriptor.nodeIndex = currentNodeIndex;
                    descriptor.parentNodeIndex = parentNodeIndex;
                    descriptor.vertexCount = static_cast<uint32_t>(outModel.meshes[flatMeshIndex].vertices.size());
                    descriptor.indexCount = static_cast<uint32_t>(outModel.meshes[flatMeshIndex].indices.size());

                    outModel.meshNodeDescriptors.push_back(descriptor);
                }
            }

            for (int childIdx : node.children) {
                queue.push({ childIdx, currentNodeIndex });
            }
        }
    }
}