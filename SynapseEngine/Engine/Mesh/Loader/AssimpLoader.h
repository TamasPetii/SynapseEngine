#pragma once
#include "Engine/SynApi.h"
#include "IMeshLoader.h"
#include <assimp/scene.h>

namespace Syn
{
	class SYN_API AssimpLoader : public IMeshLoader
	{
	public:
		AssimpLoader() = default;
		virtual std::optional<RawModel> LoadFile(const std::filesystem::path& path) override;
		virtual std::vector<std::string> GetSupportedExtensions() const override;
	private:
		void PreProcessSceneHierarchy(const aiScene* scene, RawModel& outModel);
		void ProcessSceneHierarchy(const aiScene* scene, RawModel& outModel);
		void ProcessMaterials(const aiScene* scene, RawModel& outModel);
		void ProcessMeshIndices(const aiScene* scene, RawModel& outModel);
		void ProcessMeshVertices(const aiScene* scene, RawModel& outModel);
	};
}

