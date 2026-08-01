#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include <Paingine/Graphics/Sprite.hpp>
#include <Paingine/Graphics/Texture.hpp>

// Shows a base frame (index 0) and randomly cuts to other frames for short bursts.
class GlitchEffect {
public:
    GlitchEffect();
    explicit GlitchEffect(int layer);
    ~GlitchEffect();

    // LayerManager stores raw Sprite* for the visible frame, so a copy would
    // produce two owners for one registration. Moves hand that registration over;
    // assignment unregisters whatever the target held first.
    GlitchEffect(const GlitchEffect &) = delete;
    GlitchEffect &operator=(const GlitchEffect &) = delete;
    GlitchEffect(GlitchEffect &&other) noexcept;
    GlitchEffect &operator=(GlitchEffect &&other) noexcept;

    void Stop();
    void Kill();

    void AddFrame(std::shared_ptr<Paingine2D::Texture> texture);
    void AddFrame(std::shared_ptr<Paingine2D::Sprite> sprite);
    void AddFrames(const std::vector<std::shared_ptr<Paingine2D::Sprite> > &sprites);
    void AddFrames(const std::vector<std::shared_ptr<Paingine2D::Texture> > &textures);

    void SetPosition(float x, float y);
    void SetLayer(int layer);

    // deltaTime is in seconds; pass the frame's real elapsed time.
    void Update(float deltaTime);

    void StartGlitch();
    void SetGlitchParameters(float chance, float multiChance, float duration);

private:
    static constexpr int MAX_GLITCH_FRAMES = 4;

    void RegisterToLayerManager();
    void UpdateLayerManager();
    void UnregisterCurrentFrame();
    void AppendFrame(std::shared_ptr<Paingine2D::Sprite> sprite);

    // Paingine2D::Sprite holds a non-owning const Texture*, so any texture a frame
    // was built from must be kept alive here for as long as the sprite exists.
    std::vector<std::shared_ptr<Paingine2D::Texture> > m_OwnedTextures;
    std::vector<std::shared_ptr<Paingine2D::Sprite> > m_Frames;
    std::vector<std::size_t> m_GlitchSequence; // Frames still queued for this burst
    std::size_t m_CurrentFrame = 0;
    bool m_HasRegisteredFrame = false;
    std::size_t m_RegisteredFrame = 0;
    int m_Layer = 0;
    bool m_IsRunning = true;

    bool m_IsGlitching = false;
    float m_GlitchTimer = 0.0f;
    float m_GlitchChance = 0.001f;      // Chance per Update call to start a burst
    float m_MultiGlitchChance = 0.3f;   // Chance to extend a burst by one more frame
    float m_GlitchDuration = 0.05f;     // Seconds each glitch frame shows
};
