#include "Animation/GlitchEffect.h"

#include <cstdlib>

#include "Graphics/LayerManager.h"

GlitchEffect::GlitchEffect()
    : m_Layer(0)
    , m_CurrentFrame(0)
    , m_PreviousFrame(-1)
    , m_IsRunning(true)
    , m_IsGlitching(false)
    , m_GlitchTimer(0.0f)
    , m_GlitchChance(0.001f)
    , m_MultiGlitchChance(0.3f)
    , m_GlitchDuration(0.05f) {
}

GlitchEffect::GlitchEffect(int layer)
    : m_Layer(layer)
    , m_CurrentFrame(0)
    , m_PreviousFrame(-1)
    , m_IsRunning(true)
    , m_IsGlitching(false)
    , m_GlitchTimer(0.0f)
    , m_GlitchChance(0.001f)
    , m_MultiGlitchChance(0.3f)
    , m_GlitchDuration(0.05f) {
}

GlitchEffect::~GlitchEffect() {
    for (const auto& frame : m_Frames) {
        LayerManager::RemoveDrawable(frame.get());
    }
}

void GlitchEffect::Stop() {
    m_IsRunning = false;
}

void GlitchEffect::Kill() {
    m_IsRunning = false;
    if (!m_Frames.empty() && m_CurrentFrame < static_cast<int>(m_Frames.size())) {
        LayerManager::RemoveDrawable(m_Frames[m_CurrentFrame].get());
    }
}

void GlitchEffect::AddFrame(std::shared_ptr<Paingine2D::Texture> texture) {
    m_Frames.push_back(std::make_shared<Paingine2D::Sprite>(*texture));
    if (m_Frames.size() == 1) {
        RegisterToLayerManager();
    }
}

void GlitchEffect::AddFrame(std::shared_ptr<Paingine2D::Sprite> sprite) {
    m_Frames.push_back(sprite);
    if (m_Frames.size() == 1) {
        RegisterToLayerManager();
    }
}

void GlitchEffect::AddFrames(const std::vector<std::shared_ptr<Paingine2D::Sprite>>& sprites) {
    const bool wasEmpty = m_Frames.empty();
    for (const auto& sprite : sprites) {
        m_Frames.push_back(sprite);
    }
    if (wasEmpty && !m_Frames.empty()) {
        RegisterToLayerManager();
    }
}

void GlitchEffect::AddFrames(const std::vector<std::shared_ptr<Paingine2D::Texture>>& textures) {
    const bool wasEmpty = m_Frames.empty();
    for (const auto& texture : textures) {
        m_Frames.push_back(std::make_shared<Paingine2D::Sprite>(*texture));
    }
    if (wasEmpty && !m_Frames.empty()) {
        RegisterToLayerManager();
    }
}

void GlitchEffect::SetPosition(float x, float y) {
    for (auto& sprite : m_Frames) {
        sprite->setPosition(x, y);
    }
}

void GlitchEffect::SetLayer(int layer) {
    if (m_Layer != layer) {
        if (!m_Frames.empty()) {
            LayerManager::ChangeLayer(m_Frames[m_CurrentFrame].get(), layer);
        }
        m_Layer = layer;
    }
}

void GlitchEffect::Update() {
    if (m_Frames.empty() || !m_IsRunning) {
        return;
    }

    if (m_IsGlitching) {
        m_GlitchTimer += 1.0f / 66.0f;
        if (m_GlitchTimer >= m_GlitchDuration) {
            m_PreviousFrame = m_CurrentFrame;

            if (m_GlitchSequence.empty()) {
                m_IsGlitching = false;
                m_CurrentFrame = 0;
            } else {
                m_CurrentFrame = m_GlitchSequence.back();
                m_GlitchSequence.pop_back();
                m_GlitchTimer = 0.0f;
            }
            UpdateLayerManager();
        }
    } else if (static_cast<float>(rand()) / RAND_MAX < m_GlitchChance) {
        StartGlitch();
    }
}

void GlitchEffect::RegisterToLayerManager() {
    if (!m_Frames.empty()) {
        LayerManager::AddDrawable(m_Layer, m_Frames[m_CurrentFrame].get());
        m_PreviousFrame = m_CurrentFrame;
    }
}

void GlitchEffect::UpdateLayerManager() {
    if (!m_Frames.empty()) {
        if (m_PreviousFrame >= 0 && m_PreviousFrame < static_cast<int>(m_Frames.size()) &&
            m_PreviousFrame != m_CurrentFrame) {
            LayerManager::RemoveDrawable(m_Frames[m_PreviousFrame].get());
        }

        LayerManager::AddDrawable(m_Layer, m_Frames[m_CurrentFrame].get());
        m_PreviousFrame = m_CurrentFrame;
    }
}

void GlitchEffect::StartGlitch() {
    if (m_Frames.size() <= 1) {
        return;
    }

    m_IsGlitching = true;
    m_GlitchTimer = 0.0f;
    m_GlitchSequence.clear();
    m_PreviousFrame = m_CurrentFrame;

    int numGlitchFrames = 1;
    while (static_cast<float>(rand()) / RAND_MAX < m_MultiGlitchChance && numGlitchFrames < 4) {
        numGlitchFrames++;
    }

    for (int i = 0; i < numGlitchFrames; i++) {
        int randomFrame;
        do {
            randomFrame = rand() % static_cast<int>(m_Frames.size());
        } while (randomFrame == 0);
        m_GlitchSequence.push_back(randomFrame);
    }

    if (!m_GlitchSequence.empty()) {
        m_CurrentFrame = m_GlitchSequence.back();
        m_GlitchSequence.pop_back();
        UpdateLayerManager();
    }
}

void GlitchEffect::SetGlitchParameters(float chance, float multiChance, float duration) {
    m_GlitchChance = chance;
    m_MultiGlitchChance = multiChance;
    m_GlitchDuration = duration;
}
