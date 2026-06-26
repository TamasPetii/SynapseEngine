#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/ITransformApi.h"
#include <glm/glm.hpp>

namespace Syn 
{
    using ChangePositionCommand = ComponentChangeCommand<ITransformApi, glm::vec3, &ITransformApi::SetEntityPosition>;
    using ChangeRotationCommand = ComponentChangeCommand<ITransformApi, glm::vec3, &ITransformApi::SetEntityRotation>;
    using ChangeScaleCommand    = ComponentChangeCommand<ITransformApi, glm::vec3, &ITransformApi::SetEntityScale>;
}