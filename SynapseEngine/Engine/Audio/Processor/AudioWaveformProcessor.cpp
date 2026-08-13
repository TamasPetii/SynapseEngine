// Copyright (C) 2026 Tamás Péter
// This file is part of SynapseEngine.
//
// SynapseEngine is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SynapseEngine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with SynapseEngine. If not, see <https://www.gnu.org/licenses/>.

#include "AudioWaveformProcessor.h"
#include <algorithm>

namespace Syn
{
    AudioWaveformProcessor::AudioWaveformProcessor(uint32_t resolution)
        : _resolution(resolution) {}

    void AudioWaveformProcessor::Process(CookedAudio& cookedAudio)
    {
        if (cookedAudio.samples.empty() || cookedAudio.totalFrames == 0) return;

        cookedAudio.waveform.resize(_resolution);

        uint64_t framesPerChunk = cookedAudio.totalFrames / _resolution;
        if (framesPerChunk == 0) framesPerChunk = 1;

        for (uint32_t i = 0; i < _resolution; ++i)
        {
            uint64_t startFrame = i * framesPerChunk;
            uint64_t endFrame = std::min(startFrame + framesPerChunk, cookedAudio.totalFrames);

            float minAmp = 0.0f;
            float maxAmp = 0.0f;

            for (uint64_t f = startFrame; f < endFrame; ++f)
            {
                float sample = 0.0f;

                for (uint32_t c = 0; c < cookedAudio.channels; ++c) {
                    sample += cookedAudio.samples[f * cookedAudio.channels + c];
                }
                sample /= static_cast<float>(cookedAudio.channels);

                if (sample < minAmp) minAmp = sample;
                if (sample > maxAmp) maxAmp = sample;
            }

            cookedAudio.waveform[i] = { minAmp, maxAmp };
        }
    }
}