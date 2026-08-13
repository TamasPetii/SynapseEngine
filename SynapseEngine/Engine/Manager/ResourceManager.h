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
#include "Engine/SynApi.h"
#include "PreviewManager.h"
#include <memory>
#include "Engine/Shader/ShaderManager.h"
#include "Engine/Shader/Builder/ShaderBuilder.h"

#include "Engine/Image/ImageManager.h"
#include "Engine/Mesh/ModelManager.h"
#include "Engine/Material/MaterialManager.h"

#include "Engine/Animation/AnimationManager.h"
#include "Engine/Animation/Builder/AnimationBuilder.h"

#include "Engine/Audio/Builder/AudioBuilder.h"
#include "Engine/Audio/AudioManager.h"

#include "Engine/Video/Builder/VideoBuilder.h"
#include "Engine/Video/VideoManager.h"

namespace Syn {

    class SYN_API ResourceManager {
    public:
        ResourceManager(uint32_t framesInFlight);
        ~ResourceManager();

        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
    private:
        void InitShaderManager();
        void InitModelManager();
        void InitImageManager();
        void InitMaterialManager();
        void InitAnimationManager();
        void InitPreviewManager();
        void InitAudioManager();
        void InitVideoManager(bool useGpuDecoding = true);
    private:
        std::shared_ptr<ShaderBuilder> _shaderBuilder;
        std::unique_ptr<ShaderManager> _shaderManager;

        std::unique_ptr<MaterialManager> _materialManager;

		std::shared_ptr<StaticMeshBuilder> _staticMeshBuilder;
		std::unique_ptr<ModelManager> _modelManager;

        std::shared_ptr<ImageBuilder> _imageBuilder;
        std::unique_ptr<ImageManager> _imageManager;

        std::shared_ptr<AnimationBuilder> _animationBuilder;
        std::unique_ptr<AnimationManager> _animationManager;

        std::unique_ptr<PreviewManager> _previewManager;

        std::shared_ptr<AudioBuilder> _audioBuilder;
        std::unique_ptr<AudioManager> _audioManager;

        std::shared_ptr<VideoBuilder> _videoBuilder;
        std::unique_ptr<VideoManager> _videoManager;

		uint32_t _framesInFlight;
    };
}