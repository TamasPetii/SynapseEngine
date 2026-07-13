#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/IAnimationApi.h"

namespace Syn
{
    using ChangeAnimationSpeedCommand = ComponentChangeCommand<IAnimationApi, float, &IAnimationApi::SetAnimationSpeed>;
    using ChangeAnimationIndexCommand = ComponentChangeCommand<IAnimationApi, uint32_t, &IAnimationApi::SetAnimationIndex>;
}