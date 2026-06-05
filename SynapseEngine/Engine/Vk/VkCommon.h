#pragma once
#include <volk.h>

#if __has_include(<vma/vk_mem_alloc.h>)
    #include <vma/vk_mem_alloc.h>
#else
    #include <vk_mem_alloc.h>
#endif

#include "Engine/SynApi.h"
#include "Engine/SynMacro.h"

#include <vector>
#include <memory>
#include <optional>
#include <stdexcept>
#include <span>
#include <string>
#include <functional>
#include <set>
#include <algorithm>
#include <unordered_map>
#include <map>