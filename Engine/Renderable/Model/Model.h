#pragma once
#include "Engine/Renderable/Renderable.h"
#include "Engine/Renderable/Materialized.h"
#include "Engine/Renderable/Instanceable.h"
#include "Engine/Renderable/BoundingVolume.h"
#include "Engine/Utils/AsyncLoaded.h"
#include "Engine/Utils/BufferArrayIndexed.h"

#include "AssimpConverter.h"
#include <Assimp/Importer.hpp>
#include <Assimp/postprocess.h>
#include <assimp/types.h>
#include <Assimp/scene.h>
#include <filesystem>
#include <queue>

#include "NodeTransform.h"
#include "Engine/Managers/ImageManager.h"
#include "Engine/Managers/MaterialManager.h"
#include "Engine/Components/DefaultColliderComponent.h"

class ENGINE_API Model : public Renderable, public Materialized, public Instanceable, public BoundingVolume, public AsyncLoaded, public BufferArrayIndexed
{
public:
	struct ENGINE_API MeshProcessInfo
	{
		aiMesh* mesh;
		uint32_t meshIndex;
		uint32_t vertexOffset;
		uint32_t vertexCount;
		uint32_t indexOffset;
		uint32_t indexCount;
		uint32_t nodeIndex;
	};
	struct ENGINE_API NodeTransformInfo
	{
		glm::mat4 localTransform;
		glm::mat4 globalTransform;
		glm::mat4 globalTransformIT;
	};
public:
	Model(std::shared_ptr<ImageManager> imageManager, std::shared_ptr<MaterialManager> materialManager);
	void Load(const std::string& path);
	auto GetMeshCount() { return meshCount; }
	auto GetVertexCount() { return vertexCount; }
	auto GetIndexCount() { return indexCount; }
	auto GetNodeTransformBuffer() { return nodeTransformBuffer; }
	const auto& GetMeshProcessInfos() { return meshProcessInfos; }
	const auto& GetNodeTransformInfos() { return nodeTransformInfos; }
private:
	virtual void UploadToGpu();
	virtual void PopulateSurfacePoints() override;
	void PreFetch(const aiScene* scene);
	void Process(const aiScene* scene);
	void ProcessMeshVertices(const aiScene* scene);
	void ProcessMeshVertex(const aiScene* scene, uint32_t meshIndex);
	void ProcessMeshIndices(const aiScene* scene);
	void ProcessMeshIndex(const aiScene* scene, const MeshProcessInfo& meshProcessInfo);
	void ProcessMaterials(const aiScene* scene);
	void ProcessMaterial(const aiScene* scene, uint32_t materialIndex);
	void UploadNodeTransformDataToGpu();
private:
	std::string path;
	std::string directory;
private:
	uint32_t meshCount = 0;
	std::vector<MeshProcessInfo> meshProcessInfos;
	std::vector<NodeTransformInfo> nodeTransformInfos;
	std::shared_ptr<Vk::Buffer> nodeTransformBuffer;
	std::vector<DefaultColliderComponent> meshColliderInfos;
private:
	std::shared_ptr<ImageManager> imageManager;
	std::shared_ptr<MaterialManager> materialManager;
};

