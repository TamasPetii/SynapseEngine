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

namespace Syn::Vk {
    class SYN_API Instance {
    public:
        Instance(bool enableValidation, std::span<const char*> windowExtensions);
        ~Instance();

        VkInstance Handle() const { return _handle; }
        bool IsValidationEnabled() const { return _validationEnabled; }
    private:
        void SetupVolk();
        void CreateInstance(std::span<const char*> extensions);
        void SetupDebugMessenger();
    private:
        VkInstance _handle = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
        bool _validationEnabled = false;
    };
}