#include "MiniAudioLoader.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace Syn
{
    std::optional<RawAudio> MiniAudioLoader::LoadFile(const std::filesystem::path& path)
    {
        ma_decoder decoder;
        ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
        ma_result result = ma_decoder_init_file(path.string().c_str(), &config, &decoder);

        if (result != MA_SUCCESS) {
            return std::nullopt;
        }

        RawAudio rawAudio{};
        rawAudio.channels = decoder.outputChannels;
        rawAudio.sampleRate = decoder.outputSampleRate;

        ma_uint64 estimatedFrameCount;
        ma_decoder_get_length_in_pcm_frames(&decoder, &estimatedFrameCount);

        rawAudio.samples.resize(estimatedFrameCount * decoder.outputChannels);

        ma_uint64 actualFramesRead = 0;
        ma_decoder_read_pcm_frames(&decoder, rawAudio.samples.data(), estimatedFrameCount, &actualFramesRead);

        rawAudio.totalFrames = actualFramesRead;
        rawAudio.samples.resize(actualFramesRead * decoder.outputChannels);
        rawAudio.samples.shrink_to_fit();

        ma_decoder_uninit(&decoder);

        return rawAudio;
    }

    std::optional<RawAudio> MiniAudioLoader::LoadMemory(const std::vector<uint8_t>& data)
    {
        ma_decoder decoder;
        ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
        ma_result result = ma_decoder_init_memory(data.data(), data.size(), &config, &decoder);

        if (result != MA_SUCCESS) {
            return std::nullopt;
        }

        RawAudio rawAudio{};
        rawAudio.channels = decoder.outputChannels;
        rawAudio.sampleRate = decoder.outputSampleRate;

        ma_uint64 estimatedFrameCount;
        ma_decoder_get_length_in_pcm_frames(&decoder, &estimatedFrameCount);

        rawAudio.samples.resize(estimatedFrameCount * decoder.outputChannels);

        ma_uint64 actualFramesRead = 0;
        ma_decoder_read_pcm_frames(&decoder, rawAudio.samples.data(), estimatedFrameCount, &actualFramesRead);

        rawAudio.totalFrames = actualFramesRead;
        rawAudio.samples.resize(actualFramesRead * decoder.outputChannels);
        rawAudio.samples.shrink_to_fit();

        ma_decoder_uninit(&decoder);

        return rawAudio;
    }

    std::vector<std::string> MiniAudioLoader::GetSupportedExtensions() const
    {
        return { ".wav", ".mp3", ".flac", ".ogg" };
    }
}