#include "DefaultAudioCooker.h"

namespace Syn
{
    CookedAudio DefaultAudioCooker::Cook(const RawAudio& rawAudio)
    {
        CookedAudio cookedAudio{};

        cookedAudio.channels = rawAudio.channels;
        cookedAudio.sampleRate = rawAudio.sampleRate;
        cookedAudio.totalFrames = rawAudio.totalFrames;
        cookedAudio.samples = rawAudio.samples;

        if (cookedAudio.channels == 1) {
            cookedAudio.isSpatialized = true;
        }
        else {
            cookedAudio.isSpatialized = false;
        }

        return cookedAudio;
    }
}