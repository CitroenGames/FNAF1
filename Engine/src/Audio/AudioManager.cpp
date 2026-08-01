#include "Audio/AudioManager.h"

#include <algorithm>
#include <utility>

#include "Assets/Resources.h"

AudioManager &AudioManager::GetInstance() {
    static AudioManager instance;
    return instance;
}

void AudioManager::PlayMusic(const std::string &id, bool loop, float volume) {
    // Restarting the same track should reuse its voice rather than stack a second one.
    StopMusic(id);
    ReapFinishedSounds();

    auto music = Resources::GetMusic(id);
    if (!music) {
        return;
    }

    music->setLoop(loop);
    music->setVolume(volume);
    music->play();
    m_ActiveMusic.push_back({id, std::move(music)});
}

void AudioManager::PlaySFX(const std::string &id, float volume) {
    // One-shots are allowed to overlap, so only finished voices are reclaimed.
    ReapFinishedSounds();

    auto sound = Resources::GetMusic(id);
    if (!sound) {
        return;
    }

    sound->setLoop(false);
    sound->setVolume(volume);
    sound->play();
    m_ActiveSounds.push_back({id, std::move(sound)});
}

void AudioManager::StopAllAudio() {
    for (const Voice &voice: m_ActiveMusic) {
        voice.clip->stop();
    }
    m_ActiveMusic.clear();

    for (const Voice &voice: m_ActiveSounds) {
        voice.clip->stop();
    }
    m_ActiveSounds.clear();
}

void AudioManager::StopMusic(const std::string &id) {
    const auto removed = std::remove_if(m_ActiveMusic.begin(), m_ActiveMusic.end(), [&id](const Voice &voice) {
        if (voice.id != id) {
            return false;
        }

        voice.clip->stop();
        return true;
    });

    m_ActiveMusic.erase(removed, m_ActiveMusic.end());
}

void AudioManager::ReapFinishedSounds() {
    const auto finished = std::remove_if(m_ActiveSounds.begin(), m_ActiveSounds.end(), [](const Voice &voice) {
        return voice.clip->getStatus() == AudioClip::Status::Stopped;
    });

    m_ActiveSounds.erase(finished, m_ActiveSounds.end());
}

std::size_t AudioManager::ActiveVoiceCount() const {
    return m_ActiveMusic.size() + m_ActiveSounds.size();
}
