#pragma once
#include <variant>
#include "MaterialGraphState.h"

namespace Syn 
{

    struct CreateLinkIntent {
        GraphID startPinId;
        GraphID endPinId;
    };

    struct DeleteLinkIntent {
        GraphID linkId;
    };

    using MaterialGraphIntent = std::variant<
        CreateLinkIntent,
        DeleteLinkIntent
    >;
}