#include "DefaultCpuAudioExtractor.h"

namespace Syn
{
    void DefaultCpuAudioExtractor::Extract(const CookedAudio& cookedData, CpuAudioData& outCpuData) const
    {
        outCpuData.channels = cookedData.channels;
        outCpuData.sampleRate = cookedData.sampleRate;
        outCpuData.totalFrames = cookedData.totalFrames;
        outCpuData.isSpatialized = cookedData.isSpatialized;
        outCpuData.samples = cookedData.samples;
        outCpuData.waveform = cookedData.waveform;
    }
}