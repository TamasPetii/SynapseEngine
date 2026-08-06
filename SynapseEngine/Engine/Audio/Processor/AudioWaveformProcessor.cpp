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