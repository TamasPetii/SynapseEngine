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
#include "EditorCore/Api/IModelApi.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Scene/SceneManager.h"

namespace Syn {
    class ModelApiImpl : public IModelApi {
    public:
        ModelApiImpl(ModelManager* modelManager, SceneManager* sceneManager)
            : _modelManager(modelManager), _sceneManager(sceneManager) {}

        std::vector<ModelItemData> GetAllModels() const override;
        uint64_t GetVersion() const override;

        void SetSelected(uint32_t modelId, int32_t nodeIndex) override;
        std::pair<uint32_t, int32_t> GetSelected() const override;

        const CpuModelData* GetModelCpuData(uint32_t modelId) const override;
        std::string GetNodeName(uint32_t modelId, uint16_t nodeIndex) const override;

        void ApplyModelToPreviewObject(uint32_t modelId) override;
    private:
        ModelManager* _modelManager;
        SceneManager* _sceneManager;
        uint32_t _selectedModelId = INVALID_MODEL_ID;
        int32_t _selectedNodeIndex = INVALID_NODE_INDEX;
    };
}