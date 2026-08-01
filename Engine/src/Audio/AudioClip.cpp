#include "Audio/AudioClip.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include <miniaudio.h>

#include "Utils/Log.h"

namespace {
    // Formats "<what> <name>: <miniaudio error>" consistently for every failure path.
    std::string AudioError(std::string_view what, std::string_view name, ma_result result) {
        std::string message(what);
        if (!name.empty()) {
            message += ' ';
            message += name;
        }
        message += ": ";
        message += ma_result_description(result);
        return message;
    }

    class MiniaudioRuntime {
    public:
        MiniaudioRuntime() {
            const ma_result result = ma_engine_init(nullptr, &m_Engine);
            if (result != MA_SUCCESS) {
                Log::Error("Audio", AudioError("failed to initialize miniaudio engine", "", result));
                return;
            }

            m_Initialized = true;
        }

        ma_engine *GetEngine() {
            return m_Initialized ? &m_Engine : nullptr;
        }

    private:
        ma_engine m_Engine{};
        bool m_Initialized = false;
    };

    MiniaudioRuntime &GetAudioRuntime() {
        static auto *runtime = new MiniaudioRuntime();
        return *runtime;
    }
}

struct AudioBuffer::Impl {
    std::vector<float> samples;
    ma_format format = ma_format_f32;
    ma_uint32 channels = 0;
    ma_uint32 sampleRate = 0;
    ma_uint64 frameCount = 0;
    std::string debugName;
};

std::shared_ptr<AudioBuffer> AudioBuffer::CreateFromMemory(const void *data,
                                                           std::size_t size,
                                                           const std::string &debugName) {
    if (data == nullptr || size == 0) {
        Log::Error("AudioBuffer", "empty audio data for " + debugName);
        return nullptr;
    }

    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
    ma_uint64 frameCount = 0;
    void *decodedSamples = nullptr;

    const ma_result result = ma_decode_memory(data, size, &config, &frameCount, &decodedSamples);
    if (result != MA_SUCCESS) {
        Log::Error("AudioBuffer", AudioError("failed to decode", debugName, result));
        return nullptr;
    }

    if (decodedSamples == nullptr || frameCount == 0 || config.channels == 0 || config.sampleRate == 0) {
        ma_free(decodedSamples, nullptr);
        Log::Error("AudioBuffer", "decoded no playable samples for " + debugName);
        return nullptr;
    }

    auto impl = std::make_unique<Impl>();
    impl->format = config.format;
    impl->channels = config.channels;
    impl->sampleRate = config.sampleRate;
    impl->frameCount = frameCount;
    impl->debugName = debugName;

    const std::size_t sampleCount = static_cast<std::size_t>(frameCount) * config.channels;
    impl->samples.resize(sampleCount);
    std::memcpy(impl->samples.data(), decodedSamples, sampleCount * sizeof(float));
    ma_free(decodedSamples, nullptr);

    return std::shared_ptr<AudioBuffer>(new AudioBuffer(std::move(impl)));
}

AudioBuffer::AudioBuffer(std::unique_ptr<Impl> impl)
    : m_Impl(std::move(impl)) {
}

AudioBuffer::~AudioBuffer() = default;
AudioBuffer::AudioBuffer(AudioBuffer &&) noexcept = default;
AudioBuffer &AudioBuffer::operator=(AudioBuffer &&) noexcept = default;

bool AudioBuffer::IsValid() const {
    return m_Impl && !m_Impl->samples.empty() && m_Impl->channels > 0 && m_Impl->sampleRate > 0 && m_Impl->frameCount > 0;
}

struct AudioClip::Impl {
    ma_audio_buffer dataSource{};
    ma_sound sound{};
    bool dataSourceInitialized = false;
    bool soundInitialized = false;
    std::string debugName;
};

std::shared_ptr<AudioClip> AudioClip::Create(std::shared_ptr<AudioBuffer> buffer,
                                             const std::string &debugName) {
    if (!buffer || !buffer->IsValid()) {
        return nullptr;
    }

    ma_engine *engine = GetAudioRuntime().GetEngine();
    if (engine == nullptr) {
        return nullptr;
    }

    auto impl = std::make_unique<Impl>();
    impl->debugName = debugName;

    const auto &bufferImpl = *buffer->m_Impl;
    ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(
        bufferImpl.format,
        bufferImpl.channels,
        bufferImpl.frameCount,
        bufferImpl.samples.data(),
        nullptr);
    bufferConfig.sampleRate = bufferImpl.sampleRate;

    ma_result result = ma_audio_buffer_init(&bufferConfig, &impl->dataSource);
    if (result != MA_SUCCESS) {
        Log::Error("AudioClip", AudioError("failed to create audio buffer for", debugName, result));
        return nullptr;
    }
    impl->dataSourceInitialized = true;

    result = ma_sound_init_from_data_source(
        engine,
        &impl->dataSource,
        MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_NO_PITCH,
        nullptr,
        &impl->sound);
    if (result != MA_SUCCESS) {
        ma_audio_buffer_uninit(&impl->dataSource);
        impl->dataSourceInitialized = false;
        Log::Error("AudioClip", AudioError("failed to create sound for", debugName, result));
        return nullptr;
    }
    impl->soundInitialized = true;

    return std::shared_ptr<AudioClip>(new AudioClip(std::move(buffer), std::move(impl)));
}

AudioClip::AudioClip(std::shared_ptr<AudioBuffer> buffer, std::unique_ptr<Impl> impl)
    : m_Buffer(std::move(buffer))
      , m_Impl(std::move(impl)) {
}

AudioClip::~AudioClip() {
    if (m_Impl) {
        if (m_Impl->soundInitialized) {
            ma_sound_uninit(&m_Impl->sound);
        }
        if (m_Impl->dataSourceInitialized) {
            ma_audio_buffer_uninit(&m_Impl->dataSource);
        }
    }
}

AudioClip::AudioClip(AudioClip &&) noexcept = default;
AudioClip &AudioClip::operator=(AudioClip &&) noexcept = default;

void AudioClip::play() {
    if (!IsValid()) {
        return;
    }

    ma_result result = ma_sound_stop(&m_Impl->sound);
    if (result != MA_SUCCESS) {
        Log::Error("AudioClip", AudioError("failed to stop before replaying", m_Impl->debugName, result));
        return;
    }

    result = ma_sound_seek_to_pcm_frame(&m_Impl->sound, 0);
    if (result != MA_SUCCESS) {
        Log::Error("AudioClip", AudioError("failed to rewind", m_Impl->debugName, result));
        return;
    }

    result = ma_sound_start(&m_Impl->sound);
    if (result != MA_SUCCESS) {
        Log::Error("AudioClip", AudioError("failed to start", m_Impl->debugName, result));
    }
}

void AudioClip::stop() {
    if (!IsValid()) {
        return;
    }

    ma_sound_stop(&m_Impl->sound);
    ma_sound_seek_to_pcm_frame(&m_Impl->sound, 0);
}

void AudioClip::setLoop(bool loop) {
    if (!IsValid()) {
        return;
    }

    ma_sound_set_looping(&m_Impl->sound, loop ? MA_TRUE : MA_FALSE);
}

void AudioClip::setVolume(float volume) {
    if (!IsValid()) {
        return;
    }

    const float normalizedVolume = std::clamp(volume, 0.0f, 100.0f) / 100.0f;
    ma_sound_set_volume(&m_Impl->sound, normalizedVolume);
}

AudioClip::Status AudioClip::getStatus() const {
    if (!IsValid()) {
        return Status::Stopped;
    }

    return ma_sound_is_playing(&m_Impl->sound) ? Status::Playing : Status::Stopped;
}

bool AudioClip::IsValid() const {
    return m_Impl && m_Impl->soundInitialized && m_Impl->dataSourceInitialized;
}
