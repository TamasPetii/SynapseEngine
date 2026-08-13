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

#pragma once
#ifdef VK_NO_PROTOTYPES
#include <volk.h>
#else
#include <vulkan/vulkan.h>
#endif

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