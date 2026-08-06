#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/IAnimationCompApi.h"

namespace Syn
{
    using ChangeAnimationSpeedCommand = ComponentChangeCommand<IAnimationCompApi, float, &IAnimationCompApi::SetAnimationSpeed>;
    using ChangeAnimationIndexCommand = ComponentChangeCommand<IAnimationCompApi, uint32_t, &IAnimationCompApi::SetAnimationIndex>;
}