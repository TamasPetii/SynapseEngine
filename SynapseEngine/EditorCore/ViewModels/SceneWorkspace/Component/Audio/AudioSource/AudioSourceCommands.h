#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/IAudioSourceApi.h"

namespace Syn
{
    using ChangeAudioSourceVolumeCommand = ComponentChangeCommand<IAudioSourceApi, float, &IAudioSourceApi::SetAudioSourceVolume>;
    using ChangeAudioSourcePitchCommand = ComponentChangeCommand<IAudioSourceApi, float, &IAudioSourceApi::SetAudioSourcePitch>;
    using ChangeAudioSourceMinDistanceCommand = ComponentChangeCommand<IAudioSourceApi, float, &IAudioSourceApi::SetAudioSourceMinDistance>;
    using ChangeAudioSourceMaxDistanceCommand = ComponentChangeCommand<IAudioSourceApi, float, &IAudioSourceApi::SetAudioSourceMaxDistance>;
}