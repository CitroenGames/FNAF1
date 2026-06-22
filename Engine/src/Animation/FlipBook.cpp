#include "FlipBook.h"

#include <utility>

#include "Graphics/LayerManager.h"

FlipBook::FlipBook()
    : m_Layer(0), m_FrameDuration(0), m_ElapsedTime(0.f),
    m_CurrentFrame(0), m_IsPlayingFlag(false), m_Loop(false),
    m_IsForward(true) {
}

FlipBook::~FlipBook() {
    Cleanup();
}

FlipBook::FlipBook(int layer, float frameDuration, bool loop)
    : m_Layer(layer), m_FrameDuration(frameDuration), m_ElapsedTime(0.f),
    m_CurrentFrame(0), m_IsPlayingFlag(false), m_Loop(loop),
    m_IsForward(true) {
}

void FlipBook::AddFrame(std::shared_ptr<Paingine2D::Texture> texture) {
    auto sprite = std::make_shared<Paingine2D::Sprite>(*texture);
    AddSpriteFrame(std::move(sprite));
}

void FlipBook::AddFrame(std::shared_ptr<Paingine2D::Sprite> sprite) {
    AddSpriteFrame(std::move(sprite));
}

void FlipBook::AddFrames(const std::vector<std::shared_ptr<Paingine2D::Sprite>>& sprites) {
    for (const auto& sprite : sprites) {
        AddFrame(sprite);
    }
}

void FlipBook::AddFrames(const std::vector<Paingine2D::Texture>& textures) {
    for (const auto& texture : textures) {
        AddFrame(std::make_shared<Paingine2D::Texture>(texture));
    }
}

void FlipBook::Update(float deltaTime) {
    if (!m_IsPlayingFlag || m_Frames.empty()) {
        return;
    }

    m_ElapsedTime += deltaTime;
    if (m_ElapsedTime >= m_FrameDuration) {
        m_ElapsedTime = 0.0f;

        // Store old frame index for cleanup
        size_t oldFrame = m_CurrentFrame;

        // Update frame index
        if (m_IsForward) {
            m_CurrentFrame++;
            if (m_CurrentFrame >= m_Frames.size()) {
                if (m_Loop) {
                    m_CurrentFrame = 0;
                }
                else {
                    m_CurrentFrame = m_Frames.size() - 1;
                    m_IsPlayingFlag = false;
                }
            }
        }
        else {
            if (m_CurrentFrame == 0) {
                if (m_Loop) {
                    m_CurrentFrame = m_Frames.size() - 1;
                }
                else {
                    m_IsPlayingFlag = false;
                }
            }
            else {
                m_CurrentFrame--;
            }
        }

        // Update frame registration only if needed
        if (oldFrame != m_CurrentFrame) {
            UpdateLayerManagerRegistration();
        }
    }
}

void FlipBook::RegisterToLayerManager() {
    if (HasCurrentFrame()) {
        UnregisterFromLayerManager();
        LayerManager::AddDrawable(m_Layer, m_Frames[m_CurrentFrame].get());
    }
}

void FlipBook::UnregisterFromLayerManager() {
    for (const auto& frame : m_Frames) {
        LayerManager::RemoveDrawable(frame.get());
    }
}

void FlipBook::Play(bool forward) {
    if (m_Frames.empty()) return;

    if (m_IsPlayingFlag && forward != m_IsForward) {
        UnregisterFromLayerManager();
    }

    m_IsPlayingFlag = true;
    m_IsForward = forward;

    RegisterToLayerManager();
}

void FlipBook::Pause() {
    m_IsPlayingFlag = false;
}

void FlipBook::Stop() {
    if (!m_Frames.empty()) {
        UnregisterFromLayerManager();
        m_IsPlayingFlag = false;
        m_CurrentFrame = 0;
        m_ElapsedTime = 0.f;
        RegisterToLayerManager();
    }
}

void FlipBook::Cleanup() {
    UnregisterFromLayerManager();
    m_IsPlayingFlag = false;
    m_CurrentFrame = 0;
    m_ElapsedTime = 0.f;
}

void FlipBook::SetPosition(float x, float y) {
    for (auto& frame : m_Frames) {
        frame->setPosition(x, y);
    }
}

Paingine2D::Sprite* FlipBook::GetCurrentFrame() {
    if (HasCurrentFrame()) {
        return m_Frames[m_CurrentFrame].get();
    }
    return nullptr;
}

const Paingine2D::Sprite* FlipBook::GetCurrentFrame() const {
    if (HasCurrentFrame()) {
        return m_Frames[m_CurrentFrame].get();
    }
    return nullptr;
}

void FlipBook::SetFrameDuration(float duration) {
    m_FrameDuration = duration;
}

void FlipBook::SetLoop(bool shouldLoop) {
    m_Loop = shouldLoop;
}

bool FlipBook::IsPlaying() const {
    return m_IsPlayingFlag;
}

void FlipBook::AddSpriteFrame(std::shared_ptr<Paingine2D::Sprite> sprite) {
    if (!m_Frames.empty()) {
        sprite->setPosition(m_Frames[0]->getPosition());
    }
    m_Frames.push_back(std::move(sprite));
}

bool FlipBook::HasCurrentFrame() const {
    return !m_Frames.empty() && m_CurrentFrame < m_Frames.size();
}

void FlipBook::UpdateLayerManagerRegistration() {
    UnregisterFromLayerManager();
    if (HasCurrentFrame()) {
        LayerManager::AddDrawable(m_Layer, m_Frames[m_CurrentFrame].get());
    }
}
