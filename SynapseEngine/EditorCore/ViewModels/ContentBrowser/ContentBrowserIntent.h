#pragma once
#include <string>
#include <variant>

namespace Syn 
{
    struct ChangeDirectoryIntent { 
        std::string newPath;
    };

    struct SelectEntryIntent { 
        std::string path;
    };

    struct SetThumbnailSizeIntent { 
        float newSize;
    };

    struct RefreshDirectoryIntent {

    };

    using ContentBrowserIntent = std::variant<
        ChangeDirectoryIntent,
        SelectEntryIntent,
        SetThumbnailSizeIntent,
        RefreshDirectoryIntent
    >;
}