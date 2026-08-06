#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "IApi.h"
#include "Engine/Audio/Data/Cpu/CpuAudioData.h"

namespace Syn
{
    constexpr uint32_t INVALID_AUDIO_ID = 0xFFFFFFFF;

    struct AudioItemData {
        uint32_t id;
        std::string name;
        std::string path;
    };

    class IAudioApi : public IApi {
    public:
        virtual ~IAudioApi() = default;

        virtual std::vector<AudioItemData> GetAllAudios() const = 0;
        virtual uint64_t GetVersion() const = 0;

        virtual void SetSelected(uint32_t audioId) = 0;
        virtual uint32_t GetSelected() const = 0;

        virtual const CpuAudioData* GetAudioCpuData(uint32_t audioId) const = 0;
    };
}