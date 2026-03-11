#pragma once
#include "Engine/System/ISystem.h"
#include <unordered_map>
#include <vulkan/vulkan.h>
#include "Engine/Mesh/MeshDrawDescriptor.h"

namespace Syn
{
    class SYN_API RenderSystem : public ISystem
    {
    public:
        static constexpr uint32_t MAX_INDIRECT_COMMANDS = 200000;
        static constexpr uint32_t MESHLET_OFFSET_START = 100000;

        RenderSystem();

        std::string GetName() const override { return "RenderSystem"; }
        std::vector<TypeID> GetReadDependencies() const override;
        std::vector<TypeID> GetWriteDependencies() const override { return {}; }

        void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
        void OnFinish(Scene* scene, tf::Subflow& subflow) override;
    private:
        void RebuildGlobalBuffers(Scene* scene, const std::unordered_map<uint32_t, uint32_t>& currentCounts);
    private:
        std::unordered_map<uint32_t, uint32_t> _modelCapacities;
        bool _needsRebuild = true;
        bool _needsUpload = true;

        std::vector<MeshDrawDescriptor> _descriptors;
        std::vector<VkDrawIndirectCommand> _traditionalCommands;
        std::vector<VkDrawMeshTasksIndirectCommandEXT> _meshletCommands;

        uint32_t _activeDescriptorCount = 0;
        uint32_t _activeTraditionalCount = 0;
        uint32_t _activeMeshletCount = 0;
    };
}