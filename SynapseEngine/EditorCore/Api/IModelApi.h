#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "Engine/Mesh/Data/Cpu/CpuModelData.h"

namespace Syn
{
    constexpr uint32_t INVALID_MODEL_ID = 0xFFFFFFFF;
    constexpr int32_t INVALID_NODE_INDEX = -1;

    struct ModelItemData {
        uint32_t id;
        std::string name;
        std::string path;
    };

    class IModelApi {
    public:
        virtual ~IModelApi() = default;

        virtual std::vector<ModelItemData> GetAllModels() const = 0;
        virtual uint64_t GetVersion() const = 0;

        virtual void SetSelected(uint32_t modelId, int32_t nodeIndex) = 0;
        virtual std::pair<uint32_t, int32_t> GetSelected() const = 0;

        virtual const CpuModelData* GetModelCpuData(uint32_t modelId) const = 0;
        virtual std::string GetNodeName(uint32_t modelId, uint16_t nodeIndex) const = 0;
    };
}