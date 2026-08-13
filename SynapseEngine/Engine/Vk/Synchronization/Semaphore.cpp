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

#include "Semaphore.h"
#include "Engine/ServiceLocator.h"
#include "Engine/Vk/Context.h"

namespace Syn::Vk {

    Semaphore::Semaphore() = default;

    Semaphore::~Semaphore() {
        if (_handle != VK_NULL_HANDLE) {
            auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();
            vkDestroySemaphore(device->Handle(), _handle, nullptr);
            _handle = VK_NULL_HANDLE;
        }
    }

    Semaphore::Semaphore(Semaphore&& other) noexcept {
        _handle = other._handle;
        other._handle = VK_NULL_HANDLE;
    }

    Semaphore& Semaphore::operator=(Semaphore&& other) noexcept {
        if (this != &other) {
            if (_handle != VK_NULL_HANDLE) {
                auto device = ServiceLocator::Get<Vk::Context>()->GetDevice();
                vkDestroySemaphore(device->Handle(), _handle, nullptr);
            }

            _handle = other._handle;
            other._handle = VK_NULL_HANDLE;
        }
        return *this;
    }
}