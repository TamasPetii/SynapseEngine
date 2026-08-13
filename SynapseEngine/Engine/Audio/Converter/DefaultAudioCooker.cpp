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