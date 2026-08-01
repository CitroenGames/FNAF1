#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "Audio/AudioClip.h"

class AudioManager {
public:
    static AudioManager &GetInstance();

    void PlayMusic(const std::string &id, bool loop = false, float volume = 100.0f);
    void PlaySFX(const std::string &id, float volume = 100.0f);
    void StopAllAudio();
    void StopMusic(const std::string &id);

    // Releases one-shot sounds that have finished playing. Called automatically
    // whenever a new sound starts; expose it so a scene can reclaim voices early.
    void ReapFinishedSounds();

    std::size_t ActiveVoiceCount() const;

private:
    AudioManager() = default;

    struct Voice {
        std::string id;
        std::shared_ptr<AudioClip> clip;
    };

    // Both lists own the only reference to their clips, so erasing an entry
    // frees the underlying miniaudio voice.
    std::vector<Voice> m_ActiveMusic;
    std::vector<Voice> m_ActiveSounds;
};
