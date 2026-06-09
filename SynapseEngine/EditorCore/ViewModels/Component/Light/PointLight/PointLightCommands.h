#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/IPointLightApi.h"
#include <glm/glm.hpp>

namespace Syn 
{
    using ChangePointLightColorCommand      = ComponentChangeCommand<IPointLightApi, glm::vec3, &IPointLightApi::SetLightColor>;
    using ChangePointLightStrengthCommand   = ComponentChangeCommand<IPointLightApi, float, &IPointLightApi::SetLightStrength>;
    using ChangePointLightRadiusCommand     = ComponentChangeCommand<IPointLightApi, float, &IPointLightApi::SetLightRadius>;
    using ChangePointLightWeakenCommand     = ComponentChangeCommand<IPointLightApi, float, &IPointLightApi::SetLightWeakenDistance>;
    using ChangePointLightShadowNearCommand = ComponentChangeCommand<IPointLightApi, float, &IPointLightApi::SetShadowNearPlane>;
    using ChangePointLightShadowFarCommand  = ComponentChangeCommand<IPointLightApi, float, &IPointLightApi::SetShadowFarPlane>;
}