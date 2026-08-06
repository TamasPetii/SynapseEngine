#include "MiniAudioEngine.h"
#include "Engine/Component/Audio/AudioSourceComponent.h"
#include "Engine/Logger/SynLog.h"

namespace Syn
{
    MiniAudioEngine::~MiniAudioEngine()
    {
        Shutdown();
    }

    void MiniAudioEngine::Init()
    {
        ma_engine_config config = ma_engine_config_init();
        if (ma_engine_init(&config, &_engine) != MA_SUCCESS) {
            Error("Failed to initialize Miniaudio Engine");
            return;
        }
        _isInitialized = true;
    }

    void MiniAudioEngine::Shutdown()
    {
        if (!_isInitialized) return;

        StopAllSounds();

        ma_engine_uninit(&_engine);
        _isInitialized = false;
    }

    void MiniAudioEngine::SetListenerTransform(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up)
    {
        if (!_isInitialized) return;

        ma_engine_listener_set_position(&_engine, 0, position.x, position.y, position.z);
        ma_engine_listener_set_direction(&_engine, 0, forward.x, forward.y, forward.z);
        ma_engine_listener_set_world_up(&_engine, 0, up.x, up.y, up.z);
    }

    void MiniAudioEngine::UpdateSound(EntityID entity, const AudioSourceComponent& component, const CpuAudioData& audioData, const glm::vec3& position)
    {
        if (!_isInitialized || audioData.samples.empty()) return;

        std::lock_guard lock(_mutex);

        auto it = _activeSounds.find(entity);
        if (it == _activeSounds.end()) {
            if (!component.play) return;

            ActiveSound& active = _activeSounds[entity];

            ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(
                ma_format_f32,
                audioData.channels,
                audioData.totalFrames,
                audioData.samples.data(),
                nullptr
            );
            bufferConfig.sampleRate = audioData.sampleRate;

            if (ma_audio_buffer_init(&bufferConfig, &active.buffer) != MA_SUCCESS) {
                _activeSounds.erase(entity);
                return;
            }

            ma_sound_config soundConfig = ma_sound_config_init();

            soundConfig.pDataSource = &active.buffer;

            if (!component.isSpatialized) {
                soundConfig.flags |= MA_SOUND_FLAG_NO_SPATIALIZATION;
            }

            if (ma_sound_init_ex(&_engine, &soundConfig, &active.sound) != MA_SUCCESS) {
                ma_audio_buffer_uninit(&active.buffer);
                _activeSounds.erase(entity);
                return;
            }

            active.isInitialized = true;

            ma_sound_set_attenuation_model(&active.sound, ma_attenuation_model_linear);
            ma_sound_set_looping(&active.sound, component.loop ? MA_TRUE : MA_FALSE);
            ma_sound_start(&active.sound);

            it = _activeSounds.find(entity);
        }

        auto& active = it->second;

        if (component.play) {
            if (!ma_sound_is_playing(&active.sound)) {
                if (ma_sound_at_end(&active.sound)) {
                    ma_sound_seek_to_pcm_frame(&active.sound, 0);
                }
                ma_sound_start(&active.sound);
            }
        }
        else {
            if (ma_sound_is_playing(&active.sound)) {
                ma_sound_stop(&active.sound);
            }
        }

        ma_sound_set_volume(&active.sound, component.volume);
        ma_sound_set_pitch(&active.sound, component.pitch);
        ma_sound_set_looping(&active.sound, component.loop ? MA_TRUE : MA_FALSE);

        bool isCurrentlySpatialized = (ma_sound_is_spatialization_enabled(&active.sound) == MA_TRUE);

        if (isCurrentlySpatialized != component.isSpatialized) {
            ma_sound_set_spatialization_enabled(&active.sound, component.isSpatialized ? MA_TRUE : MA_FALSE);
        }

        if (component.isSpatialized) {
            ma_sound_set_position(&active.sound, position.x, position.y, position.z);
            ma_sound_set_min_distance(&active.sound, component.minDistance);
            ma_sound_set_max_distance(&active.sound, component.maxDistance);
            ma_sound_set_attenuation_model(&active.sound, ma_attenuation_model_linear);
        }
    }

    void MiniAudioEngine::StopSound(EntityID entity)
    {
        std::lock_guard lock(_mutex);
        auto it = _activeSounds.find(entity);
        if (it != _activeSounds.end()) {
            if (it->second.isInitialized) {
                ma_sound_stop(&it->second.sound);
                ma_sound_uninit(&it->second.sound);
                ma_audio_buffer_uninit(&it->second.buffer);
            }
            _activeSounds.erase(it);
        }
    }

    void MiniAudioEngine::StopAllSounds()
    {
        if (!_isInitialized) return;

        std::lock_guard lock(_mutex);
        for (auto& [entity, active] : _activeSounds) {
            if (active.isInitialized) {
                ma_sound_stop(&active.sound);
                ma_sound_uninit(&active.sound);
                ma_audio_buffer_uninit(&active.buffer);
            }
        }
        _activeSounds.clear();
    }
}