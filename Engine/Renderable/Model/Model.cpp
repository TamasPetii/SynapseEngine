#include "Model.h"
#include "Engine/Timer/Timer.h"
#include <algorithm>
#include <execution>
#include <ranges>
#include <future>
#include <thread>
#include "Engine/Vulkan/VulkanContext.h"

Model::Model(std::shared_ptr<ImageManager> imageManager, uint32_t addressArrayIndex) :
    imageManager(imageManager),
    addressArrayIndex(addressArrayIndex)
{
}

void Model::Load(const std::string& path)
{
    std::cout << std::format("[Thread Started] : Loading model path={}", path) << "\n";

    Timer timer;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        state = LoadState::Failed;
        return ;
    }

    std::cout << "-----------------------------------------\n";
    std::cout << "Assimp::Importer: " << timer.GetElapsedTime() << "\n";
    std::cout << "-----------------------------------------\n";

    this->path = path;
    this->directory = path.substr(0, path.find_last_of('/'));

    PreFetch(scene);
    Process(scene);
    GenerateBoundingVolume(aabbMin, aabbMax);

    state = LoadState::CpuLoaded;

    UploadToGpu();

    state = LoadState::GpuUploaded;
}

void Model::UploadToGpu()
{
    auto device = Vk::VulkanContext::GetContext()->GetDevice();

    Renderable::UploadToGpu();
    Model::UploadNodeTransformDataToGpu();
    Materialized::UploadMaterialDataToGpu();
}

void Model::PopulateSurfacePoints()
{
    surfacePoints.reserve(vertexCount);

    for (auto& vertex : vertices)
        surfacePoints.push_back(vertex.position);
}

void Model::PreFetch(const aiScene* scene)
{
    Timer timer;

    std::queue<std::pair<aiNode*, uint32_t>> queue;
    queue.push({ scene->mRootNode, UINT32_MAX });

    while (!queue.empty())
    {
        auto [currentNode, parentNodeIndex] = queue.front();
        queue.pop();

        uint32_t nodeIndex = nodeTransformInfos.size();

        for (uint32_t i = 0; i < currentNode->mNumMeshes; ++i)
        {
            aiMesh* currentMesh = scene->mMeshes[currentNode->mMeshes[i]];
           
            uint32_t meshVertexOffset = vertexCount;
            uint32_t meshVertexCount = currentMesh->mNumVertices;

            uint32_t meshIndexOffset = indexCount;
            uint32_t meshIndexCount = currentMesh->mNumFaces * 3;

            MeshProcessInfo meshProcessInfo{};
            meshProcessInfo.mesh = currentMesh;
            meshProcessInfo.meshIndex = meshCount;
            meshProcessInfo.vertexOffset = meshVertexOffset;
            meshProcessInfo.vertexCount = meshVertexCount;
            meshProcessInfo.indexOffset = meshIndexOffset;
            meshProcessInfo.indexCount = meshIndexCount;
            meshProcessInfo.nodeIndex = nodeIndex;
            meshProcessInfos.push_back(meshProcessInfo);

            vertexCount += meshVertexCount;
            indexCount += meshIndexCount;
            meshCount++;
        }

        glm::mat4 nodeTransform = Assimp::ConvertAssimpToGlm(currentNode->mTransformation);
        glm::mat4 parentTransform = parentNodeIndex != UINT32_MAX ? nodeTransformInfos[parentNodeIndex].globalTransform : glm::mat4(1);

        NodeTransformInfo nodeTransformInfo;
        nodeTransformInfo.localTransform = nodeTransform;
        nodeTransformInfo.globalTransform = parentTransform * nodeTransformInfo.localTransform;
        nodeTransformInfo.globalTransformIT = glm::transpose(glm::inverse(nodeTransformInfo.globalTransform));
        nodeTransformInfos.push_back(nodeTransformInfo);

        for (uint32_t i = 0; i < currentNode->mNumChildren; ++i)
            queue.push({ currentNode->mChildren[i], nodeIndex });
    }

    vertices.resize(vertexCount);
    indices.resize(indexCount);
    materials.resize(scene->mNumMaterials);

    aabbMax = glm::vec3(std::numeric_limits<float>::lowest());
    aabbMin = glm::vec3(std::numeric_limits<float>::max());

    meshColliderInfos.resize(meshProcessInfos.size());

    std::cout << "-----------------------------------------\n";
    std::cout << "PreFetch: " << timer.GetElapsedTime() << "\n";
    std::cout << "NodeSize: " << nodeTransformInfos.size() << "\n";
    std::cout << "-----------------------------------------\n";
}

void Model::Process(const aiScene* scene)
{
    auto futureMaterials = std::async(std::launch::async, &Model::ProcessMaterials, this, scene);
    auto futureVertices = std::async(std::launch::async, &Model::ProcessMeshVertices, this, scene);
    auto futureIndices = std::async(std::launch::async, &Model::ProcessMeshIndices, this, scene);

    futureMaterials.get();
    futureVertices.get();
    futureIndices.get();
}

void Model::ProcessMeshVertices(const aiScene* scene)
{
    Timer timer;

    auto mesh_index_range = std::views::iota(0u, static_cast<uint32_t>(meshProcessInfos.size()));
    std::for_each(std::execution::seq, mesh_index_range.begin(), mesh_index_range.end(),
        [&](auto meshIndex) -> void {
            ProcessMeshVertex(scene, meshIndex);
        }
    );

    std::cout << "-----------------------------------------\n";
    std::cout << "ProcessMeshVertices: " << timer.GetElapsedTime() << "\n";
    std::cout << "-----------------------------------------\n";
}

void Model::ProcessMeshVertex(const aiScene* scene, uint32_t meshIndex)
{
    const MeshProcessInfo& meshProcessInfo = meshProcessInfos[meshIndex];

    glm::vec3 meshAabbMax = glm::vec3(std::numeric_limits<float>::lowest());
    glm::vec3 meshAabbMin = glm::vec3(std::numeric_limits<float>::max());

    auto vertex_index_range = std::views::iota(0u, meshProcessInfo.vertexCount);
    std::for_each(std::execution::seq, vertex_index_range.begin(), vertex_index_range.end(),
        [&](auto vertexIndex) -> void {
            //Position
            glm::vec3 position{ 0,0,0 };
            if (meshProcessInfo.mesh->mVertices)
            {
                position = Assimp::ConvertAssimpToGlm(meshProcessInfo.mesh->mVertices[vertexIndex]);
                glm::vec3 transformedPosition = glm::vec3(nodeTransformInfos[meshProcessInfo.nodeIndex].globalTransform * glm::vec4(position, 1.0f));

                aabbMax = glm::max(aabbMax, transformedPosition);
                aabbMin = glm::min(aabbMin, transformedPosition);

                meshAabbMax = glm::max(meshAabbMax, transformedPosition);
                meshAabbMin = glm::min(meshAabbMin, transformedPosition);
            }

            //Normals
            glm::vec3 normal{ 0,0,0 };
            if (meshProcessInfo.mesh->mNormals)
            {
                normal = Assimp::ConvertAssimpToGlm(meshProcessInfo.mesh->mNormals[vertexIndex]);
            }

            //Tangents
            glm::vec3 tangent{ 0,0,0 };
            if (meshProcessInfo.mesh->mTangents)
            {
                tangent = Assimp::ConvertAssimpToGlm(meshProcessInfo.mesh->mTangents[vertexIndex]);
            }

            //Tangents
            glm::vec3 bitangent{ 0,0,0 };
            if (meshProcessInfo.mesh->mTangents)
            {
                bitangent = Assimp::ConvertAssimpToGlm(meshProcessInfo.mesh->mBitangents[vertexIndex]);
            }

            //Textures
            glm::vec2 uv{ 0,0 };
            if (meshProcessInfo.mesh->mTextureCoords[0] != nullptr)
            {
                uv.x = meshProcessInfo.mesh->mTextureCoords[0][vertexIndex].x;
                uv.y = meshProcessInfo.mesh->mTextureCoords[0][vertexIndex].y;
            }

            vertices[meshProcessInfo.vertexOffset + vertexIndex] = Vertex(position, normal, uv, tangent, bitangent, meshProcessInfo.mesh->mMaterialIndex, meshProcessInfo.nodeIndex);
        }
    );

    DefaultColliderComponent defaultCollider;
    defaultCollider.obbPositions[0] = glm::vec3(meshAabbMax.x, meshAabbMax.y, meshAabbMax.z);
    defaultCollider.obbPositions[1] = glm::vec3(meshAabbMax.x, meshAabbMax.y, meshAabbMin.z);
    defaultCollider.obbPositions[2] = glm::vec3(meshAabbMax.x, meshAabbMin.y, meshAabbMax.z);
    defaultCollider.obbPositions[3] = glm::vec3(meshAabbMax.x, meshAabbMin.y, meshAabbMin.z);
    defaultCollider.obbPositions[4] = glm::vec3(meshAabbMin.x, meshAabbMax.y, meshAabbMax.z);
    defaultCollider.obbPositions[5] = glm::vec3(meshAabbMin.x, meshAabbMax.y, meshAabbMin.z);
    defaultCollider.obbPositions[6] = glm::vec3(meshAabbMin.x, meshAabbMin.y, meshAabbMax.z);
    defaultCollider.obbPositions[7] = glm::vec3(meshAabbMin.x, meshAabbMin.y, meshAabbMin.z);
    defaultCollider.aabbMax = meshAabbMax;
    defaultCollider.aabbMin = meshAabbMin;
    defaultCollider.aabbOrigin = 0.5f * (meshAabbMin + meshAabbMax);
    defaultCollider.aabbExtents = 0.5f * (meshAabbMax - meshAabbMin);
    defaultCollider.origin = aabbOrigin;
    defaultCollider.radius = glm::max(defaultCollider.aabbExtents.x, glm::max(defaultCollider.aabbExtents.y, defaultCollider.aabbExtents.z));
    meshColliderInfos[meshIndex] = defaultCollider;
}

void Model::ProcessMeshIndices(const aiScene* scene)
{
    Timer timer;

    std::for_each(std::execution::seq, meshProcessInfos.begin(), meshProcessInfos.end(), 
        [&](const auto& meshProcessInfo) -> void {
            ProcessMeshIndex(scene, meshProcessInfo);
        }
    );

    std::cout << "-----------------------------------------\n";
    std::cout << "ProcessMeshIndices: " << timer.GetElapsedTime() << "\n";
    std::cout << "-----------------------------------------\n";
}

void Model::ProcessMeshIndex(const aiScene* scene, const MeshProcessInfo& meshProcessInfo)
{
    constexpr uint32_t faceIndexCount = 3;
    auto index_range = std::views::iota(0u, meshProcessInfo.indexCount);
    std::for_each(std::execution::seq, index_range.begin(), index_range.end(),
        [&](auto flatIndex) -> void {
            uint32_t faceIndex = flatIndex / faceIndexCount;
            uint32_t localIndex = flatIndex % faceIndexCount;

            indices[meshProcessInfo.indexOffset + flatIndex] = meshProcessInfo.vertexOffset + meshProcessInfo.mesh->mFaces[faceIndex].mIndices[localIndex];
        }
    );
}

void Model::ProcessMaterials(const aiScene* scene)
{
    Timer timer;

    auto material_index_range = std::views::iota(0u, (uint32_t)materials.size());
    std::for_each(std::execution::seq, material_index_range.begin(), material_index_range.end(), 
        [&](auto materialIndex) -> void {
            ProcessMaterial(scene, materialIndex);
        }
    );

    std::cout << "-----------------------------------------\n";
    std::cout << "ProcessMaterials: " << timer.GetElapsedTime() << "\n";
    std::cout << "-----------------------------------------\n";
}

void Model::ProcessMaterial(const aiScene* scene, uint32_t materialIndex)
{
    aiMaterial* material = scene->mMaterials[materialIndex];
    MaterialComponent materialComponent;

    //Diffuse texture
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString path;
        unsigned int uvIndex = 0;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, &uvIndex);
        std::string real_path = directory + "/" + std::string(path.C_Str());
        materialComponent.albedoTexture = imageManager->LoadImage(real_path, true);
    }

    //Normals texture
    if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
    {
        aiString path;
        unsigned int uvIndex = 0;
        material->GetTexture(aiTextureType_NORMALS, 0, &path, NULL, &uvIndex);
        std::string real_path = directory + "/" + std::string(path.C_Str());
        materialComponent.normalTexture = imageManager->LoadImage(real_path, false);
    }

    //Height texture
    if (material->GetTextureCount(aiTextureType_HEIGHT) > 0 && materialComponent.normalTexture == nullptr)
    {
        aiString path;
        unsigned int uvIndex = 0;
        material->GetTexture(aiTextureType_HEIGHT, 0, &path, NULL, &uvIndex);
        std::string real_path = directory + "/" + std::string(path.C_Str());
        materialComponent.normalTexture = imageManager->LoadImage(real_path, true);
    }

    //Displacement
    if (material->GetTextureCount(aiTextureType_DISPLACEMENT) > 0 && materialComponent.normalTexture == nullptr)
    {
        aiString path;
        unsigned int uvIndex = 0;
        material->GetTexture(aiTextureType_DISPLACEMENT, 0, &path, NULL, &uvIndex);
        std::string real_path = directory + "/" + std::string(path.C_Str());
        materialComponent.normalTexture = imageManager->LoadImage(real_path, true);
    }

    //Metallic Texture
    if (material->GetTextureCount(aiTextureType_METALNESS) > 0)
    {
        aiString path;
        unsigned int uvIndex = 0;
        material->GetTexture(aiTextureType_METALNESS, 0, &path, NULL, &uvIndex);
        std::string real_path = directory + "/" + std::string(path.C_Str());
        materialComponent.metalnessTexture = imageManager->LoadImage(real_path, true);
    }

    //Roughness Texture
    if (material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
    {
        aiString path;
        unsigned int uvIndex = 0;
        material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &path, NULL, &uvIndex);
        std::string real_path = directory + "/" + std::string(path.C_Str());
        materialComponent.roughnessTexture = imageManager->LoadImage(real_path, true);
    }

    aiColor3D diffuseColor(1.f, 1.f, 1.f);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
    materialComponent.color = glm::vec4(Assimp::ConvertAssimpToGlm(diffuseColor), 1);

    materials[materialIndex] = materialComponent;
}

void Model::UploadNodeTransformDataToGpu()
{
    VkDeviceSize nodeBufferSize = sizeof(NodeTransform) * nodeTransformInfos.size();

    Vk::BufferConfig nodeStagingConfig;
    nodeStagingConfig.size = nodeBufferSize;
    nodeStagingConfig.usage = VK_BUFFER_USAGE_2_TRANSFER_SRC_BIT;
    nodeStagingConfig.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    Vk::Buffer nodeStagingBuffer{ nodeStagingConfig };

    NodeTransform* nodeStagigngBufferHandler = static_cast<NodeTransform*>(nodeStagingBuffer.MapMemory());
    for (uint32_t i = 0; i < nodeTransformInfos.size(); ++i)
    {
        nodeStagigngBufferHandler[i].transform = nodeTransformInfos[i].globalTransform;
        nodeStagigngBufferHandler[i].transformIT = nodeTransformInfos[i].globalTransformIT;
    }
    nodeStagingBuffer.UnmapMemory();

    Vk::BufferConfig nodeBufferConfig;
    nodeBufferConfig.size = nodeBufferSize;
    nodeBufferConfig.usage = VK_BUFFER_USAGE_2_TRANSFER_DST_BIT | VK_BUFFER_USAGE_2_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_2_SHADER_DEVICE_ADDRESS_BIT;
    nodeBufferConfig.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    nodeTransformBuffer = std::make_shared<Vk::Buffer>(nodeBufferConfig);

    Vk::VulkanContext::GetContext()->GetImmediateQueue()->SubmitTransfer(
        [&](VkCommandBuffer commandBuffer) -> void {
            Vk::Buffer::CopyBufferToBuffer(commandBuffer, nodeStagingBuffer.Value(), nodeTransformBuffer->Value(), nodeBufferSize);
        }
    );
}
