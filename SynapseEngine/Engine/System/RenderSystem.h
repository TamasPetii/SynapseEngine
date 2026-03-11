#pragma once
#include "Engine/System/ISystem.h"
#include <unordered_map>
#include <vulkan/vulkan.h>
#include "Engine/Mesh/MeshDrawDescriptor.h"
#include "Engine/Mesh/MeshAllocationInfo.h"

namespace Syn
{
    class SYN_API RenderSystem : public ISystem
    {
    public:
        std::string GetName() const override { return "RenderSystem"; }
        std::vector<TypeID> GetReadDependencies() const override;

        void OnUpdate(Scene* scene, uint32_t frameIndex, float deltaTime, tf::Subflow& subflow) override;
        void OnUploadToGpu(Scene* scene, uint32_t frameIndex, tf::Subflow& subflow) override;
        void OnFinish(Scene* scene, tf::Subflow& subflow) override;

        const ModelAllocationInfo* GetModelAllocation(uint32_t modelId) const;
    private:
        void RebuildGlobalBuffers(Scene* scene);
    private:
        std::vector<uint32_t> _currentCounts;
        std::vector<uint32_t> _modelCapacities;

        bool _needsRebuild = true;
        uint32_t _framesToUpload = 0;
    };
}