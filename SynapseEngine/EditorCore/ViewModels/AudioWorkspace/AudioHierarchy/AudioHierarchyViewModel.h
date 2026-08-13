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
#include "EditorCore/ViewModels/IViewModel.h"
#include "AudioHierarchyState.h"
#include "AudioHierarchyIntent.h"
#include "EditorCore/Api/IAudioApi.h"
#include "EditorCore/Api/IPreviewApi.h"

namespace Syn {
    class AudioHierarchyViewModel : public IViewModel<AudioHierarchyState, AudioHierarchyIntent> {
    public:
        AudioHierarchyViewModel(IAudioApi* audioApi, IPreviewApi* previewApi);
        ~AudioHierarchyViewModel() override = default;

        const AudioHierarchyState& GetState() const override { return _state; }
        void SyncWithEngine() override;
        void Dispatch(const AudioHierarchyIntent& intent) override;

    private:
        void RebuildList();
    private:
        IAudioApi* _audioApi = nullptr;
        IPreviewApi* _previewApi = nullptr;
        AudioHierarchyState _state;

        bool _isDirty = true;
        uint64_t _lastEngineVersion = 0;
    };
}