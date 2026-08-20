#pragma once
#include "Engine/SynApi.h"
#include "Engine/Video/Source/IVideoSource.h"
#include "Engine/Video/Loader/IVideoLoader.h"
#include <string>
#include <memory>

namespace Syn
{
    class SYN_API NetworkVideoSource : public IVideoSource
    {
    public:
        NetworkVideoSource(const std::string& url, IVideoLoader* loader);
        ~NetworkVideoSource() override = default;

        VideoInfo GetInfo() const override;
        std::optional<RawVideoPacket> ReadNextPacket() override;
        void Reset() override;
    private:
        std::string _url;
        IVideoLoader* _loader;
        std::unique_ptr<IVideoState> _state;
    };
}