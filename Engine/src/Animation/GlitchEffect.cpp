#include "Animation/GlitchEffect.h"

#include <algorithm>
#include <random>
#include <utility>

#include "Graphics/LayerManager.h"
#include "Utils/Log.h"

namespace {
    // Shared by every effect instance. Replaces rand(), which has poor low-bit
    // quality and whose `rand() % n` is biased when n does not divide RAND_MAX+1.
    std::mt19937 &Rng() {
        static std::mt19937 engine{std::random_device{}()};
        return engine;
    }

    float RandomUnit() {
        static std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
        return distribution(Rng());
    }

    // Uniform index in [1, frameCount); frame 0 is the non-glitched base frame.
    std::size_t RandomGlitchFrame(std::size_t frameCount) {
        std::uniform_int_distribution<std::size_t> distribution(1, frameCount - 1);
        return distribution(Rng());
    }
}

GlitchEffect::GlitchEffect() = default;

GlitchEffect::GlitchEffect(int layer)
    : m_Layer(layer) {
}

GlitchEffect::~GlitchEffect() {
    UnregisterCurrentFrame();
}

// Frames are heap-allocated behind shared_ptr, so their addresses survive the
// move and any LayerManager registration stays valid under the new owner.
GlitchEffect::GlitchEffect(GlitchEffect &&other) noexcept
    : m_OwnedTextures(std::move(other.m_OwnedTextures))
    , m_Frames(std::move(other.m_Frames))
    , m_GlitchSequence(std::move(other.m_GlitchSequence))
    , m_CurrentFrame(other.m_CurrentFrame)
    , m_HasRegisteredFrame(other.m_HasRegisteredFrame)
    , m_RegisteredFrame(other.m_RegisteredFrame)
    , m_Layer(other.m_Layer)
    , m_IsRunning(other.m_IsRunning)
    , m_IsGlitching(other.m_IsGlitching)
    , m_GlitchTimer(other.m_GlitchTimer)
    , m_GlitchChance(other.m_GlitchChance)
    , m_MultiGlitchChance(other.m_MultiGlitchChance)
    , m_GlitchDuration(other.m_GlitchDuration) {
    other.m_OwnedTextures.clear();
    other.m_Frames.clear();
    other.m_GlitchSequence.clear();
    other.m_HasRegisteredFrame = false;
    other.m_IsGlitching = false;
}

GlitchEffect &GlitchEffect::operator=(GlitchEffect &&other) noexcept {
    if (this == &other) {
        return *this;
    }

    // Drop our own registration before the sprites backing it go away.
    UnregisterCurrentFrame();

    m_OwnedTextures = std::move(other.m_OwnedTextures);
    m_Frames = std::move(other.m_Frames);
    m_GlitchSequence = std::move(other.m_GlitchSequence);
    m_CurrentFrame = other.m_CurrentFrame;
    m_HasRegisteredFrame = other.m_HasRegisteredFrame;
    m_RegisteredFrame = other.m_RegisteredFrame;
    m_Layer = other.m_Layer;
    m_IsRunning = other.m_IsRunning;
    m_IsGlitching = other.m_IsGlitching;
    m_GlitchTimer = other.m_GlitchTimer;
    m_GlitchChance = other.m_GlitchChance;
    m_MultiGlitchChance = other.m_MultiGlitchChance;
    m_GlitchDuration = other.m_GlitchDuration;

    other.m_OwnedTextures.clear();
    other.m_Frames.clear();
    other.m_GlitchSequence.clear();
    other.m_HasRegisteredFrame = false;
    other.m_IsGlitching = false;
    return *this;
}

void GlitchEffect::Stop() {
    m_IsRunning = false;
}

void GlitchEffect::Kill() {
    m_IsRunning = false;
    UnregisterCurrentFrame();
}

void GlitchEffect::AddFrame(std::shared_ptr<Paingine2D::Texture> texture) {
    if (!texture) {
        Log::Error("GlitchEffect", "ignoring null texture frame");
        return;
    }

    auto sprite = std::make_shared<Paingine2D::Sprite>(*texture);
    m_OwnedTextures.push_back(std::move(texture));
    AppendFrame(std::move(sprite));
}

void GlitchEffect::AddFrame(std::shared_ptr<Paingine2D::Sprite> sprite) {
    if (!sprite) {
        Log::Error("GlitchEffect", "ignoring null sprite frame");
        return;
    }

    AppendFrame(std::move(sprite));
}

void GlitchEffect::AddFrames(const std::vector<std::shared_ptr<Paingine2D::Sprite> > &sprites) {
    for (const auto &sprite: sprites) {
        AddFrame(sprite);
    }
}

void GlitchEffect::AddFrames(const std::vector<std::shared_ptr<Paingine2D::Texture> > &textures) {
    for (const auto &texture: textures) {
        AddFrame(texture);
    }
}

void GlitchEffect::SetPosition(float x, float y) {
    for (auto &sprite: m_Frames) {
        sprite->setPosition(x, y);
    }
}

// Callers use SetLayer both to move the effect and to bring it back after Kill(),
// so a layer change re-registers the current frame when nothing is registered.
void GlitchEffect::SetLayer(int layer) {
    if (m_Layer == layer) {
        return;
    }

    m_Layer = layer;
    if (m_Frames.empty()) {
        return;
    }

    if (m_HasRegisteredFrame) {
        LayerManager::ChangeLayer(m_Frames[m_RegisteredFrame].get(), m_Layer);
        return;
    }

    RegisterToLayerManager();
}

void GlitchEffect::Update(float deltaTime) {
    if (m_Frames.empty() || !m_IsRunning) {
        return;
    }

    if (!m_IsGlitching) {
        if (RandomUnit() < m_GlitchChance) {
            StartGlitch();
        }
        return;
    }

    m_GlitchTimer += deltaTime;
    if (m_GlitchTimer < m_GlitchDuration) {
        return;
    }

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

void GlitchEffect::StartGlitch() {
    if (m_Frames.size() <= 1) {
        return;
    }

    m_IsGlitching = true;
    m_GlitchTimer = 0.0f;
    m_GlitchSequence.clear();

    int burstLength = 1;
    while (burstLength < MAX_GLITCH_FRAMES && RandomUnit() < m_MultiGlitchChance) {
        burstLength++;
    }

    m_GlitchSequence.reserve(static_cast<std::size_t>(burstLength));
    for (int i = 0; i < burstLength; ++i) {
        m_GlitchSequence.push_back(RandomGlitchFrame(m_Frames.size()));
    }

    m_CurrentFrame = m_GlitchSequence.back();
    m_GlitchSequence.pop_back();
    UpdateLayerManager();
}

void GlitchEffect::SetGlitchParameters(float chance, float multiChance, float duration) {
    m_GlitchChance = chance;
    m_MultiGlitchChance = multiChance;
    m_GlitchDuration = std::max(duration, 0.0f);
}

void GlitchEffect::AppendFrame(std::shared_ptr<Paingine2D::Sprite> sprite) {
    m_Frames.push_back(std::move(sprite));
    if (m_Frames.size() == 1) {
        RegisterToLayerManager();
    }
}

void GlitchEffect::RegisterToLayerManager() {
    if (m_Frames.empty()) {
        return;
    }

    LayerManager::AddDrawable(m_Layer, m_Frames[m_CurrentFrame].get());
    m_HasRegisteredFrame = true;
    m_RegisteredFrame = m_CurrentFrame;
}

void GlitchEffect::UpdateLayerManager() {
    if (m_Frames.empty() || (m_HasRegisteredFrame && m_RegisteredFrame == m_CurrentFrame)) {
        return;
    }

    UnregisterCurrentFrame();
    RegisterToLayerManager();
}

void GlitchEffect::UnregisterCurrentFrame() {
    if (!m_HasRegisteredFrame) {
        return;
    }

    if (m_RegisteredFrame < m_Frames.size()) {
        LayerManager::RemoveDrawable(m_Frames[m_RegisteredFrame].get());
    }

    m_HasRegisteredFrame = false;
}
