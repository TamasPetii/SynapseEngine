#pragma once
#include "EditorCore/Command/ComponentChangeCommand.h"
#include "EditorCore/Api/IModelComponentApi.h"

namespace Syn
{
    using ChangeModelCastShadowCommand = ComponentChangeCommand<IModelComponentApi, bool, &IModelComponentApi::SetCastShadow>;
    using ChangeModelReceiveShadowCommand = ComponentChangeCommand<IModelComponentApi, bool, &IModelComponentApi::SetReceiveShadow>;
    using ChangeModelIndexCommand = ComponentChangeCommand<IModelComponentApi, uint32_t, &IModelComponentApi::SetModelIndex>;
}