#pragma once
#include "Engine/EngineApi.h"
#include "Engine/Utils/AsyncLoaded.h"
#include "Engine/Vulkan/Buffer.h"
#include "Bone.h"

#include <glm/glm.hpp>
#include <string>

#include <Assimp/Importer.hpp>
#include <Assimp/postprocess.h>
#include <assimp/types.h>
#include <Assimp/scene.h>

#include "Engine/Components/DefaultColliderComponent.h"
#include "Engine/Utils/BufferArrayIndexed.h"

class ENGINE_API Animation : public AsyncLoaded, public BufferArrayIndexed
{
public:
	struct ENGINE_API VertexBoneData
	{
		glm::uvec4 indices = { UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
		glm::vec4 weights = { 0.f, 0.f, 0.f, 0.f };
	};
	struct ENGINE_API MeshProcessInfo
	{
		aiMesh* mesh;
		uint32_t meshIndex;
		uint32_t vertexOffset;
		uint32_t vertexCount;
	};
	struct ENGINE_API BoneProcessInfo
	{
		Bone bone;
		std::string name;
		glm::mat4 offsetMatrix;
	};
	struct ENGINE_API NodeProcessInfo
	{
		std::string name;
		uint32_t boneIndex = UINT32_MAX;
		uint32_t parentIndex = UINT32_MAX;
	};
public:
	Animation() = default;
	void Load(const std::string& path);
	auto GetDuration() { return duration; }
	auto GetTicksPerSeconds() { return ticksPerSecond; }
	auto GetVertexCount() { return vertexCount; }
	auto GetBoneCount() { return boneCount; }
	auto GetMeshCount() { return meshCount; }
	const auto& GetBoneIndexMap() { return boneIndexMap; }
	const auto& GetNodeProcessInfo() { return nodeProcessInfos; }
	const auto& GetBoneProcessInfo() { return boneProcessInfos; }
	const auto& GetVertexBoneBuffer() { return vertexBoneBuffer; }
private:
	void PreFetch(const aiScene* scene);
	void ProcessBoneKeyFrames(const aiScene* scene);
	void ProcessMeshVertexBones();
	void ProcessMeshVertexBone(const MeshProcessInfo& meshProcessInfo);
	void InitVertexBoneBuffer();

	double duration;
	double ticksPerSecond;

	uint32_t meshCount = 0;
	uint32_t boneCount = 0;
	uint32_t vertexCount = 0;
	std::vector<VertexBoneData> vertexBoneDatas;
	std::shared_ptr<Vk::Buffer> vertexBoneBuffer;
	std::vector<NodeProcessInfo> nodeProcessInfos;
	std::vector<MeshProcessInfo> meshProcessInfos;
	std::vector<BoneProcessInfo> boneProcessInfos;
	std::unordered_map<std::string, uint32_t> boneIndexMap;
	std::unordered_map<std::string, uint32_t> nodeIndexMap;
private:
	void InitBoundingVolumeBuffer();
	std::shared_ptr<Vk::Buffer> frameBoundingVolumesBuffer;
	std::vector<DefaultColliderComponent> frameBoundingVolumes;
};

