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
#include "../VkCommon.h"
#include <mutex>

namespace Syn::Vk 
{
    class SYN_API ThreadSafeQueue 
    {
    public:
        ThreadSafeQueue(VkQueue handle, uint32_t familyIndex);

        void Submit(VkSubmitInfo2* submitInfo, VkFence fence);
        VkQueue Handle() const { return _handle; }
        uint32_t GetFamilyIndex() const { return _familyIndex; }
    private:
        VkQueue _handle = VK_NULL_HANDLE;
        uint32_t _familyIndex = 0;
        std::mutex _mtx;
    };
}