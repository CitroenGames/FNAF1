#pragma once

#include <memory>
#include <vector>

#include <Paingine/Graphics/Sprite.hpp>
#include <Paingine/Graphics/Texture.hpp>

class FlipBook {
public:
    static constexpr float DEFAULT_FRAME_DURATION = 0.1f;
    // Floor for the per-frame duration; see SetFrameDuration.
    static constexpr float MIN_FRAME_DURATION = 1.0f / 1000.0f;

    FlipBook();
    ~FlipBook();
    FlipBook(int layer, float frameDuration = DEFAULT_FRAME_DURATION, bool loop = true);

    // LayerManager stores raw Sprite* for the registered frame, so a copy would
    // produce two owners for one registration. Moves are supported and hand off
    // that registration; assignment unregisters whatever the target held first.
    FlipBook(const FlipBook &) = delete;
    FlipBook &operator=(const FlipBook &) = delete;
    FlipBook(FlipBook &&other) noexcept;
    FlipBook &operator=(FlipBook &&other) noexcept;

    // Add frames to the flipbook
    void AddFrame(std::shared_ptr<Paingine2D::Texture> texture);
    void AddFrame(std::shared_ptr<Paingine2D::Sprite> sprite);
    void AddFrames(const std::vector<std::shared_ptr<Paingine2D::Sprite>>& sprites);
    void AddFrames(const std::vector<Paingine2D::Texture>& textures);

    // Animation control
    void Update(float deltaTime);
    void Play(bool forward = true);
    void Pause();
    void Stop();
    void Cleanup();

    // Layer management
    void RegisterToLayerManager();
    void UnregisterFromLayerManager();

    // Setters
    void SetPosition(float x, float y);
    void SetFrameDuration(float duration);
    void SetLoop(bool shouldLoop);

    // Getters
    bool IsPlaying() const;
    Paingine2D::Sprite* GetCurrentFrame();
    const Paingine2D::Sprite* GetCurrentFrame() const;

private:
    // Paingine2D::Sprite holds a non-owning const Texture*, so any texture a frame
    // was built from must be kept alive here for as long as the sprite exists.
    std::vector<std::shared_ptr<Paingine2D::Texture>> m_OwnedTextures;
    std::vector<std::shared_ptr<Paingine2D::Sprite>> m_Frames;
    float m_FrameDuration;
    float m_ElapsedTime;
    std::size_t m_CurrentFrame;
    bool m_IsPlayingFlag;
    bool m_Loop;
    int m_Layer;
    bool m_IsForward;

    void AddSpriteFrame(std::shared_ptr<Paingine2D::Sprite> sprite);
    bool HasCurrentFrame() const;
    void UpdateLayerManagerRegistration();
};
