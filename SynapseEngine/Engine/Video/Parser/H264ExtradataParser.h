#pragma once
#include "Engine/SynApi.h"
#include "IH264ExtradataParser.h"
#include <vector>
#include <cstdint>

namespace Syn
{
    class SYN_API H264ExtradataParser : public IH264ExtradataParser
    {
    public:
        H264ExtradataParser() = default;
        ~H264ExtradataParser() override = default;

        bool Parse(const std::vector<uint8_t>& extradata, StdVideoH264SequenceParameterSet& outSps, StdVideoH264PictureParameterSet& outPps) override;
    };
}