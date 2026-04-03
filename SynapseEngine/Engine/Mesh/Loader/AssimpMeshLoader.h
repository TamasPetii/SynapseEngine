#pragma once
#include "Engine/SynApi.h"
#include "IMeshLoader.h"
#include <assimp/scene.h>
#include <taskflow/taskflow.hpp>

namespace Syn
{
	class SYN_API AssimpMeshLoader : public IMeshLoader
	{
	public:
		AssimpMeshLoader() = default;
		virtual std::optional<RawModel> LoadFile(const std::filesystem::path& path) override;
		virtual std::vector<std::string> GetSupportedExtensions() const override;
	private:
		void PreProcessSceneHierarchy(const aiScene* scene, RawModel& outModel);
		void ProcessSceneHierarchy(const aiScene* scene, RawModel& outModel);
		void ProcessMaterials(const aiScene* scene, RawModel& outModel, tf::Taskflow& taskflow);
		void ProcessMeshIndices(const aiScene* scene, RawModel& outModel, tf::Taskflow& taskflow);
		void ProcessMeshVertices(const aiScene* scene, RawModel& outModel, tf::Taskflow& taskflow);
	};
}

